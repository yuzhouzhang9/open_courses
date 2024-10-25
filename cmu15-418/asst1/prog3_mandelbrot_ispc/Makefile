# 编译器设置
CXX = g++ -m64
CXXFLAGS = -I../common -Iobjs/ -O3 -Wall -std=c++11
ISPC = ispc
ISPCFLAGS = -O2 --target=avx1-i32x8 --arch=x86-64

# 可执行文件名称
APP_NAME = mandelbrot_ispc
PART3_APP = part3

OBJDIR = objs
COMMONDIR = ../common

# 其他对象文件（如 PPM 和 TaskSys 相关）
PPM_CXX = $(COMMONDIR)/ppm.cpp
PPM_OBJ = $(addprefix $(OBJDIR)/, $(subst $(COMMONDIR)/,, $(PPM_CXX:.cpp=.o)))

TASKSYS_CXX = $(COMMONDIR)/tasksys.cpp
TASKSYS_LIB = -lpthread
TASKSYS_OBJ = $(addprefix $(OBJDIR)/, $(subst $(COMMONDIR)/,, $(TASKSYS_CXX:.cpp=.o)))

# ISPC 生成的对象文件
ISPC_OBJ = $(OBJDIR)/mandelbrot_ispc.o

# ThreadPool 对象文件
THREADPOOL_OBJ = $(OBJDIR)/threadpool.o

# 所有对象文件
OBJS = $(OBJDIR)/main.o $(OBJDIR)/mandelbrotSerial.o $(OBJDIR)/mandelbrot_ispc.o \
       $(PPM_OBJ) $(TASKSYS_OBJ) $(THREADPOOL_OBJ)

PART3_OBJS = $(OBJDIR)/part3.o $(THREADPOOL_OBJ) $(OBJDIR)/mandelbrotSerial.o $(OBJDIR)/mandelbrot_ispc.o \
       	$(PPM_OBJ) $(TASKSYS_OBJ)  # part3 的对象文件

# 默认目标
default: $(APP_NAME) $(PART3_APP)

# 创建对象目录
.PHONY: dirs clean

dirs:
	/bin/mkdir -p $(OBJDIR)/

# 清理构建文件
clean:
	/bin/rm -rf $(OBJDIR) *.ppm *~ $(APP_NAME)

# 链接最终可执行文件
$(APP_NAME): dirs $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $(OBJS) -lm $(TASKSYS_LIB)

# 链接 part3 的可执行文件
$(PART3_APP): dirs $(PART3_OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $(PART3_OBJS) -lm $(TASKSYS_LIB)

# 规则：编译 .cpp 文件为 .o 文件
$(OBJDIR)/%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJDIR)/%.o: $(COMMONDIR)/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# 编译 ThreadPool.cpp
$(OBJDIR)/threadpool.o: threadpool.cpp threadpool.h
	$(CXX) $(CXXFLAGS) -c threadpool.cpp -o $(OBJDIR)/threadpool.o

# ISPC 文件的编译规则
$(OBJDIR)/%_ispc.h $(OBJDIR)/%_ispc.o: %.ispc
	$(ISPC) $(ISPCFLAGS) $< -o $(OBJDIR)/$*_ispc.o -h $(OBJDIR)/$*_ispc.h

# 依赖关系：确保 main.cpp 正确依赖 ISPC 的头文件
$(OBJDIR)/main.o: $(OBJDIR)/mandelbrot_ispc.h $(COMMONDIR)/CycleTimer.h
