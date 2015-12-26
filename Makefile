INSTALL_PATH ?= /usr/local
INSTALL_INCLUDE_PATH = $(INSTALL_PATH)/include/gungnir

INSTALL = cp -R

install:
	mkdir -p $(INSTALL_INCLUDE_PATH)
	$(INSTALL) include/gungnir/ $(INSTALL_INCLUDE_PATH)

uninstall:
	rm -rf $(INSTALL_INCLUDE_PATH)

check:
	cd tests && cmake . && make && ./test_all
