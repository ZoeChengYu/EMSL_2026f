# 編譯器與參數設定
CXX = g++
CXXFLAGS = -std=c++11 -D_USE_MATH_DEFINES

# 原始碼與目標檔列表
SRC = circular_sector.cpp ellipse.cpp main.cpp sort.cpp triangle.cpp
OBJ = $(SRC:.cpp=.o)
EXE = main

# 跨平台設定（支援 Windows 與 Linux / Mac 評測環境）
ifeq ($(OS),Windows_NT)
	RM = del /Q /F
	TARGET = $(EXE).exe
else
	RM = rm -f
	TARGET = $(EXE)
endif

# 預設目標
all: $(TARGET)

$(TARGET): $(OBJ)
	$(CXX) -o $@ $(OBJ)

# 支援直接以 $(EXE) 為目標（例如 make main）
$(EXE): $(TARGET)

# 編譯個別 .cpp 為 .o
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# 清理編譯產物
clean:
ifeq ($(OS),Windows_NT)
	-$(RM) $(EXE) $(EXE).exe $(OBJ) 2>nul
else
	$(RM) $(EXE) $(OBJ)
endif

.PHONY: all clean
