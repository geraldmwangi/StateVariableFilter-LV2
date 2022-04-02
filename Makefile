NAME= statevf
SRC= src
BUILD=build
#LV2DIR=~/.lv2
LV2URI=http://github.com/geraldmwangi/StateVF

CFILES=$(SRC)/$(NAME).c

all: $(NAME)
	


$(NAME):
	mkdir -p $(BUILD)
	$(CC) -shared -o $(BUILD)/$(NAME).so $(CFLAGS) $(CFILES)

clean:
	rm -r $(BUILD)

install: all
ifdef LV2DIR
	mkdir -p $(LV2DIR)/$(NAME).lv2
	cp $(BUILD)/$(NAME).so $(LV2DIR)/$(NAME).lv2/
	cp manifest.ttl.in $(LV2DIR)/$(NAME).lv2/manifest.ttl
	cp $(NAME).ttl $(LV2DIR)/$(NAME).lv2/$(NAME).ttl
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
