/* StateVF Library
 * Copyright (C) 2022 Gerald Mwangi
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General
 * Public License along with this program; if not, write to the
 * Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA  02110-1301  USA
 */

// Include standard libs
#define _USE_MATH_DEFINES
#include <math.h>
#include <stdlib.h>
#include <string.h>

// Include lv2core
#include <lv2/lv2plug.in/ns/lv2core/lv2.h>
#include <lv2/options/options.h>
#include <lv2/urid/urid.h>

// LV2 URI
#define STATEVF_URI "http://github.com/geraldmwangi/StateVF"

typedef struct
{
    LV2_URID atom_URID;
    LV2_URID lv2_AudioPort;
    LV2_URID lv2_CVPort;
    LV2_URID lv2_ControlPort;
    LV2_URID morph_currentType;
} URIs;

// Define port index. This corresponds to the ports section in statevf.ttl
typedef enum
{
    STATEVF_CUTOFF = 0,
    STATEVF_DAMPING = 1,
    STATEVF_INPUT = 2,
    STATEVF_HIGHPASS = 3,
    STATEVF_BANDPASS = 4,
    STATEVF_LOWPASS = 5
} PortIndex;

// The state variable filter instance, which contains the port buffers and the delay lines
typedef struct
{
    // ControlPort buffers
    const float *cutoff;
    const float *damping;

    // AudioPort buffers
    const float *input;
    float *highpass;
    float *bandpass;
    float *lowpass;
    float m_delayIn;
    float m_delayLow;
    float m_delayBand;
    float m_delayHigh;
    float m_F;
    double sample_rate;

    URIs uris;

} StateVF;

/**
 * @brief Initialize the StateVF plugin
 *
 * @param descriptor plugin descriptor with the function pointers to the API of this plugin
 * @param rate audio sample rate
 * @param bundle_path the path to the lv2 bundle
 * @param features extra host provided features
 * @return LV2_Handle
 */
static LV2_Handle instantiate(const LV2_Descriptor *descriptor,
                              double rate,
                              const char *bundle_path,
                              const LV2_Feature *const *features)
{
    // Allocate zero initialized memory
    StateVF *statevf = (StateVF *)calloc(1, sizeof(StateVF));
    if (statevf)
        statevf->sample_rate = rate;

    return (LV2_Handle)statevf;
}

/**
 * @brief connect the LV2 ports to the hosts buffers
 *
 * @param instance The current plugin instance
 * @param port The port to be connected
 * @param data the memory part to which the port is connected
 */
static void connect_port(LV2_Handle instance,
                         uint32_t port,
                         void *data)
{
    StateVF *statevf = (StateVF *)instance;
    switch ((PortIndex)port)
    {
    case STATEVF_CUTOFF:
        statevf->cutoff = (const float *)data;
        break;
    case STATEVF_DAMPING:
        statevf->damping = (const float *)data;
        break;
    case STATEVF_INPUT:
        statevf->input = (const float *)data;
        break;
    case STATEVF_HIGHPASS:
        statevf->highpass = (float *)data;
        break;
    case STATEVF_BANDPASS:
        statevf->bandpass = (float *)data;
        break;
    case STATEVF_LOWPASS:
        statevf->lowpass = (float *)data;
        break;

    default:
        break;
    }
}

/**
 * @brief Initialize the internal plugin state
 *
 * @param instance the plugin instance
 *
 */
static void activate(LV2_Handle instance)
{
    StateVF *statevf = (StateVF *)instance;
    statevf->m_delayBand = 0;
    statevf->m_delayHigh = 0;
    statevf->m_delayIn = 0;
    statevf->m_F = 2.0 * sin(M_PI * (*statevf->cutoff) / statevf->sample_rate);
}

/**
 * @brief Main process function
 *
 * @param instance Current plugin instance
 * @param nframes The number of frames in the audio port buffers
 */
static void run(LV2_Handle instance, uint32_t nframes)
{
    StateVF *statevf = (StateVF *)instance;
    statevf->m_F = 2.0 * sin(M_PI * (*statevf->cutoff) / statevf->sample_rate);

    float Q = 2.0 * (*statevf->damping);
    statevf->highpass[0] = statevf->input[0] - statevf->m_delayLow - Q * statevf->m_delayBand;
    statevf->bandpass[0] = statevf->m_F * statevf->highpass[0] + statevf->m_delayBand;
    statevf->lowpass[0] = statevf->m_F * statevf->bandpass[0] + statevf->m_delayLow;

    for (int i = 1; i < nframes; i++)
    {
        statevf->highpass[i] = statevf->input[i] - statevf->lowpass[i - 1] - Q * statevf->bandpass[i - 1];
        statevf->bandpass[i] = statevf->m_F * statevf->highpass[i] + statevf->bandpass[i - 1];
        statevf->lowpass[i] = statevf->m_F * statevf->bandpass[i] + statevf->lowpass[i - 1];
    }

    statevf->m_delayHigh = statevf->highpass[nframes - 1];
    statevf->m_delayBand = statevf->bandpass[nframes - 1];
    statevf->m_delayLow = statevf->lowpass[nframes - 1];
}

/**
 * @brief reset internal plugin state, free resources which where allocated in activate()
 *
 * @param instance Current plugin instance
 */
static void deactivate(LV2_Handle instance)
{
}

/**
 * @brief deallocate the plugin
 *
 * @param instance Current plugin instance
 */
static void cleanup(LV2_Handle instance)
{
    free(instance);
}

static uint32_t
options_set(LV2_Handle instance,
            const LV2_Options_Option *options)
{
    StateVF *plugin = (StateVF *)instance;
    uint32_t ret = 0;
    for (const LV2_Options_Option *o = options; o->key; ++o)
    {
        if (o->context != LV2_OPTIONS_PORT)
        {
            ret |= LV2_OPTIONS_ERR_BAD_SUBJECT;
        }
        else if (o->key != plugin->uris.morph_currentType)
        {
            ret |= LV2_OPTIONS_ERR_BAD_KEY;
        }
        else if (o->type != plugin->uris.atom_URID)
        {
            ret |= LV2_OPTIONS_ERR_BAD_VALUE;
        }
        else
        {
            LV2_URID port_type = *(const LV2_URID *)(o->value);
            if (port_type != plugin->uris.lv2_ControlPort &&
                port_type != plugin->uris.lv2_CVPort)
            {
                ret |= LV2_OPTIONS_ERR_BAD_VALUE;
                continue;
            }

            switch (o->subject)
            {
            case STATEVF_CUTOFF:
                // plugin->cutoff_is_cv = (port_type == plugin->uris.lv2_CVPort);
                break;
            case STATEVF_DAMPING:
                // plugin->resonance_is_cv = (port_type == plugin->uris.lv2_CVPort);
                break;
            default:
                ret |= LV2_OPTIONS_ERR_BAD_SUBJECT;
            }
        }
    }
    return ret;
}

/**
 * @brief NI
 *
 * @param uri
 * @return const void*
 */
static const void *extension_data(const char *uri)
{
    static const LV2_Options_Interface options = {NULL, options_set};
    if (!strcmp(uri, LV2_OPTIONS__interface))
    {
        return &options;
    }
    return NULL;
}

static const LV2_Descriptor descriptor = {
    STATEVF_URI,
    instantiate,
    connect_port,
    activate,
    run,
    deactivate,
    cleanup,
    extension_data};

/**
 * @brief Main entrypoint to the library
 *
 * @param index
 * @return LV2_SYMBOL_EXPORT const*
 */
LV2_SYMBOL_EXPORT
const LV2_Descriptor *
lv2_descriptor(uint32_t index)
{
    switch (index)
    {
    case 0:
        return &descriptor;
    default:
        return NULL;
    }
}