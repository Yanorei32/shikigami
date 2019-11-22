CSC		= /cygdrive/c/Windows/Microsoft.NET/Framework/v4.0.30319/csc.exe
TARGET	= tcpclient.exe
SRC		= main.cs
DEPS	=

FRAMEWORK_DIR_W	= C:\\Windows\\Microsoft.NET\\Framework\\v4.0.30319\\
CSC_FLAGS		= /nologo \
	/utf8output

DEBUG_FLAGS		= 
RELEASE_FLAGS	= /t:winexe

.PHONY: debug
debug: CSC_FLAGS+=$(DEBUG_FLAGS)
debug: all

.PHONY: release
release: CSC_FLAGS+=$(RELEASE_FLAGS)
release: all

all: $(TARGET)
$(TARGET): $(SRC) $(DEPS)
	$(CSC) $(CSC_FLAGS) /out:$(TARGET) $(SRC)

.PHONY: clean
clean:
	rm $(TARGET)

