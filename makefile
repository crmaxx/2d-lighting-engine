BIN = ./bin
SRC = ./src

all:
	$(MAKE) -sC $(SRC) all

clean:
	rm -rf $(BIN) && cd $(SRC) && $(MAKE) -s clean
