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

.PHONY: all run clean test run4 run5 png package

# 範例：4/5 變數 PLA 測試
run4: all
	./$(TARGET_EXEC) pla_files/my4.pla my4.dot

run5: all
	./$(TARGET_EXEC) pla_files/my5.pla my5.dot

# 轉出 PNG（需安裝 Graphviz）
png:
	@which dot >/dev/null 2>&1 || { echo "Graphviz 未安裝(apt install graphviz)"; exit 0; }
	-dot -Tpng output_obdd.dot -o obdd_graph.png
	-dot -Tpng output.dot -o graph.png
	-dot -Tpng my4_obdd.dot -o my4_obdd.png
	-dot -Tpng my4.dot -o my4_robdd.png
	-dot -Tpng my5_obdd.dot -o my5_obdd.png
	-dot -Tpng my5.dot -o my5_robdd.png

# 打包提交（請先把 report.pdf 放在根目錄）
package:
	mkdir -p submit
	cp -r src Makefile pla_files README.md submit/
	cp -n *.dot *.png submit/ 2>/dev/null || true
	[ -f report.pdf ] && cp report.pdf submit/ || true
	tar czf B1330031.tgz -C submit .
	@echo "打包完成:B1330031.tgz"