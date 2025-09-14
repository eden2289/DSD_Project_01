# --------------------
# Makefile for ROBDD Project (修改版：執行檔輸出至根目錄)
# --------------------

# 1. Compiler and Flags
CXX = g++
CXXFLAGS = -std=c++17 -g -Wall

# 2. Project Structure
SRCS := $(wildcard src/*.cpp)  # 自動尋找src資料夾下所有的.cpp檔案
TARGET_EXEC = robdd            # 最終的執行檔名稱 (直接在根目錄)

# 3. Rules

# 預設規則：輸入 "make" 或 "make all" 就會執行這裡
all: $(TARGET_EXEC)

# 編譯規則：告訴make如何從.cpp檔案生成執行檔
$(TARGET_EXEC): $(SRCS)
	# 因為不再需要build資料夾，所以移除了 @mkdir 指令
	$(CXX) $(CXXFLAGS) -o $@ $^

# 執行規則：可以用 "make run" 來編譯並執行
run: all
	./$(TARGET_EXEC) $(ARGS)

# 測試規則：用 "make test" 執行預設的輸入檔案
test: all
	./$(TARGET_EXEC) pla_files/input.pla output.dot

# 清理規則：用 "make clean" 來刪除編譯好的檔案
clean:
	# 將清理目標從 build 資料夾改為清理執行檔本身
	@rm -f $(TARGET_EXEC)

.PHONY: all run clean test