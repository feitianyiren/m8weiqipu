# m8weiqipu
运行于魅族M8手机上的简单实用的围棋打谱软件

M8围棋谱 v0.8.8

为魅族M8手机开发的围棋打谱软件，还在开发过程中，目前功能不全，但基本功能可用。现在仅支持SGF格式的围棋棋谱。

基本功能：打开并解析SGF棋谱，单步向前向后，落子音效，自动提子，显示棋局信息和解说，支持自定义皮肤。

作者：liigo，http://blog.csdn.net/liigo，http://blog.csdn.net/liigo/article/details/4700244

此软件是 Google Code 上的开源软件，项目主页为：http://code.google.com/p/m8weiqipu/

请移步魅族M8官方论坛下载：http://bbs.meizu.com/viewthread.php?tid=1250668

![](http://hi.csdn.net/attachment/200910/19/11443_1255956224y877.png)



### 2010/07/18更新(v0.8.8)，增加新功能，为进入魅族软件中心做出多处修改，首次登陆魅族软件中心：

- 支持棋谱中的预置棋子（多见于让子棋、死活棋）；
- 支持显示黑方先行还是白方先行；
- 程序启动时还原上次棋局状态前给予提示；
- 打谱到最后一子之后或第一子之前时给予提示；
- 支持在英文系统下读取含中文信息的SGF棋谱文件，并保证正常显示汉字不乱码；
- 单实例运行；
- 建立USB连接时自动静默退出；
- 增加程序启动、退出、窗口切换时的动画效果；
- 响应音量键调整落子音效音量；

### 2010/04/14更新(0.8.7)，增加新功能并修正BUG：

- 退出程序时记录当前打谱进度，下次启动程序后自动还原棋局状态；
- 修正设置界面中底部工具栏超出屏幕区域的BUG；

### 2010/03/06更新(v0.8.6)，增加新功能并修正BUG：

- 不再全屏，显示出任务栏（信号栏）以便于切换后台任务；
- 在棋盘右上角显示当前落子方、棋子序号及其位置；
- 支持多子快速打谱（前十手、后十手）；
- 完善SGF解析器，支持弃权落子；
- 修正“提子后后退一步可能导致程序崩溃”的BUG；

### 2010/03/03更新(v0.8.5)，支持M8新UI固件(0.9.6.12)：

- 此版本仅支持新UI固件，使用旧UI固件的朋友请继续使用0.8.3版本；
- 更换了默认皮肤，采用新的棋盘和棋子图片；
- 响应新UI编程规范，调整单击M键的功能为退出程序；

### 2009/10/22更新(v0.8.3)，修正一个BUG：

- 修正“从默认标准棋盘切换到19路小棋盘再切换到9路小棋盘，会导致解说显示错位”的BUG，感谢魅友liigo哈哈；

### 2009/10/20更新(v0.8.2)，修正一个BUG：

- 修正“关闭了声音但是开始打谱后第一下还是有声音”的BUG，感谢魅友炸鸡翅；
- 压缩包中去除了无用的.svn目录，感谢魅友926m8；

### 2009/10/19更新(v0.8.1)，修正一个BUG：

- 设置界面中“每次关闭声音后回到打谱界面就没有棋盘了”的BUG，感谢魅友音乐树；

### 2009/10/18更新(v0.8)，很多新功能*：

- 显示棋局信息（对弈双方、赛事、日期、胜负、总落子数等）；
- 显示解说信息，以及与解说相关的标注；
- 突出显示最新落子的棋子；
- 允许关闭或打开落子音效；
- 允许设定“下一子”按钮在左边还是在右边；
- 支持自定义皮肤，可以随时切换；
- 更换了软件图标（感谢魅友Isaeva）；
- 默认皮肤修改为棕色背景图片（感谢魅友gjgj666666）；
- 优化或重新制作了部分原有皮肤图片，以及配置文件；
- 窗口激活时隐藏信息栏，保证始终全屏；
- 提供了皮肤设计说明书，参见 http://docs.google.com/View?id=dhmvxcsd_3dxpmm6nz 或 http://blog.csdn.net/liigo/article/details/4705305

### 2009/09/29更新(v0.3)：

- 修正SGF解析器中一处可能导致死循环的BUG（感谢魅友dozen在70楼提供导致程序出错的棋谱）；
- 允许手工修改主配置文件(M8WeiqiPu.ini)切换皮肤；

### 2009/09/27更新(v0.2)：

　　最近两个晚上紧急修正了SGF解析器中的一个BUG，此BUG可能会导致程序崩溃甚至死机。感谢魅友dozen在56楼提供的导致程序出错的三个棋谱，感谢魅友gjgj666666在60楼提供的导致程序出错的大约一千个棋谱（当然不一定都出错，我没具体测试），所有这些棋谱都通过了新版软件测试（当然是程序测试，人工测试还不把我累死?），出错的机率已经很小了（在我的M8上只是程序无故退出，从未死机，固件0.9.2.7）。

　　替换了落子音效文件，调低了音量。

　　最近很忙，其它功能暂时没时间加了，以后再慢慢完善，反正现在的功能已基本满足我日常使用的最低要求了（能看谱）。
