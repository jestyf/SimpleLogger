# SimpleLogger
一个使用纯C编写的低延迟日志库。  
readme in English: [readme_en](./README_en.md)

## 如何使用
1. 添加源码，引入头文件  
首先将src下的所有源码放到工程中，然后引入`simple_logger.h`这个头文件。  
```C
#define LOG_TAG "slog"
#include "simple_logger.h"
```
这里可以在引入头文件前，为每个.c/.cpp文件设置tag，tag可以用来标示不同的模块和文件，也可以用来标示整个项目。如果不设置，tag将不会被显示。  

2. 启动日志器  
执行以下函数。
```C
slog_start();
```

3. 输出日志  
使用`slog_x(x is level)`来输出日志，使用方法就像使用printf一样。注意，这里不需要在结尾添加`"\n"`.   
```
// log level is VERBOSE
slog_v(fmt,...);
// log level is DEBUG
slog_d(fmt,...);
// log level is INFO
slog_i(fmt,...);
// log level is WARNING
slog_w(fmt,...);
// log level is ERROR
slog_e(fmt,...);
```
4. 停止日志器  
请务必在程序退出之前关闭日志器，否则可能输出会不完全。  
```C
slog_stop();
```
## 日志库的设置

- slog_set_file
    - 设置输出文件的路径。默认是`/tmp/slog.log`。日志文件达到行数上限时会自动添加编号。如果设置为NULL，则表示关闭文件输出功能。  
- slog_set_output_to_file
    - 设置是否启动文件输出。如果在`slog_set_file`中设置NULL，则文件输出关闭，这一函数无效。  
- slog_set_content_for_each_level  
    - 为每个日志等级设置输出内容  

    |contents|label|
    |:--:|:--:|
    |日志等级|LOG_CONTENT_level|
    |时间|LOG_CONTENT_TIME|
    |代码文件名、行、函数名|LOG_CONTENT_CODE_INFO|
    |线程号、进程号|LOG_CONTENT_THREAD_INFO|
    |标签，如果不设置则不显示|LOG_CONTENT_TAG|
    |全部内容|LOG_CONTENT_ALL|

    可以使用这样的方式来设置:
    ```C
    slog_set_content_for_each_level(LOG_LEVEL_INFO,LOG_CONTENT_ALL&(~LOG_CONTENT_CODE_INFO));
    ```
    默认情况，输出全部内容。  

- slog_set_color_output  
    彩色输出。设置成1则终端中打印的日志是彩色的。注意：文件中保存的日志不是，也不会有彩色输出相关的控制字。  
- slog_set_level  
    日志等级。低于这个等级将会被过滤掉。  
- slog_set_stdout_redirection  
    标准输出重定向。可以重定向到stderr等。如果被设置为NULL，则是关闭控制台输出。 

## 设计  
这里使用了一个环形队列来保存每个日志。然后启动一个新的线程来处理队列中的日志。这样可以避免生成日志时有大量的IO时间消耗。  
## 延迟测试  
延迟测试如下：  
![slog benchmark](./image/slog_bench.jpg)  
此外，我也测试了另一个纯C编写的日志库, [EasyLogger](https://github.com/armink/EasyLogger)  
![EasyLogger benchmark](./image/elog_bench.jpg)  

## 参考  
本项目参考了以下工程的代码  
EasyLogger: <https://github.com/armink/EasyLogger>  
NanoLogger: <https://github.com/Iyengar111/NanoLog>  
向以上项目的作者表示真诚的感谢！  