NAME= statevf
SRC= src
BUILD=build
#LV2DIR=~/.lv2
LV2URI=http://github.com/geraldmwangi/StateVF

CFILES=$(SRC)/$(NAME).c

COMPILEFLAGS= $(CFLAGS) -std=c99 -std=gnu99 -fPIC -g


all: $(NAME)
	


$(NAME):
	mkdir -p $(BUILD)
	$(CC) $(COMPILEFLAGS)  -shared -o $(BUILD)/$(NAME).so  $(CFILES)

clean:
	rm -r $(BUILD)

install: all
ifdef LV2DIR
	mkdir -p $(LV2DIR)/$(NAME).lv2
	cp $(BUILD)/$(NAME).so $(LV2DIR)/$(NAME).lv2/
	cp manifest.ttl $(LV2DIR)/$(NAME).lv2/manifest.ttl
	cp $(NAME).ttl $(LV2DIR)/$(NAME).lv2/$(NAME).ttl
	cp modgui.ttl $(LV2DIR)/$(NAME).lv2/modgui.ttl
	cp -R modgui $(LV2DIR)/$(NAME).lv2/
else
	echo "LV2DIR not specified. Run LV2DIR=/path/to/lv2plugins make install"
endif

uninstall:
ifdef LV2DIR
	rm -r $(LV2DIR)/$(NAME).lv2
else
	echo "LV2DIR not specified. Run LV2DIR=/path/to/lv2plugins make uninstall"
endif
test: install
	jalv.qt5 $(LV2URI)
