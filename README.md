# StateVF
A state variable filter. This is a filter which output a highpass, bandpass and lowpass signal.
The damping factor allows for resonant sounds when set near zero. TREAT WITH CAUTION, YOU CAN DAMAGE YOU EARS!


## Build
You need the lv2 development headers and libraries. On ubuntu (or any debian derivativ) run
```bash
sudo apt-get install lv2-dev
```
then build
```bash
make
```

## Install
You must tell the installer where the lv2 bundle is to be installed
```bash
LV2DIR=/path/to/lv2plugins make install
```

For uninstalling run
```bash
LV2DIR=/path/to/lv2plugins make unstall
```

## Test
You must have jalv.qt5 installed to run the test.
First start jackd (I use qjackctl as the frontend)
Run
```bash
LV2DIR=/path/to/lv2plugins make test
```