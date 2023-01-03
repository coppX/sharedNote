# 文件说明
- SoundWave: 音波文件，由原始的.wav格式音频导入到UE里面形成的uasset文件
- SoundCue：音效对象，内部播放的是SoundWave文件，内部可能会使用多个SoundWave文件来实现特定的播放效果，比如从多个SoundWare中随机播放一个，建议播放音效的时候尽量使用音效对象
- SoundClass：音效类，可以用来将音效划分类型，对这一类型的所有音效进行统一管理，配合被动混音修改器一起使用。音效类可以用来对SoundWave和SoundCue进行分类
- SoundClassMix：音效类混合，对不同类型音效进行混合的时候的混合规则，比如音量在混合的时候，类型A和类型B的音量分别怎么变化
- SoundSubMix: 音效子混合
- SoundConcurrency：音效并发设置
- SoundSourceBus: 音源总线

# 播放方式
## 蓝图
![](./img/1.png)
这个是只管播放一个声音，触发播放了就没法再对这个声音进行控制了
![](./img/2.png)
这是先创建一个AudioComponent对象，然后我们对这个对象进行操作，包括播放，停止，修改音量，淡入淡出等。
## notify
![](./img/3.png)
![](./img/4.png)
这种方式是在动画蒙太奇里面播放音效，通过向track添加播放音效的通知，就能控制在动画执行的某一帧上播放一个音效。

# 音量设置

# 混音修改器
## 配置规则
在使用混音修改器之前，我们需要建立混音规则，需要创建SoundClass和SoundClassMix。
新建SoundClass：Content Browser空白处右键->Sounds->Classes->Sound Class，就能新建一个SoundClass，我们可以对多个SoundClass建立树形结构。比如像下面这种
![](./img/6.png)
新建SoundClassMix: Content Browser空白处右键->Sounds->Classes->Sound Class Mix
接下来就是创建这个混合规则，我们看这个例子
![](./img/7.png)
这个名为DuckforDialogue的混合，里面Sound Class Effects里面添加了三个类型，说明这个音效类混合是针对这三个类型的音频来进行混合的，其中Music和SFX类型的Sound的音量会变成原来的0.2, Dialogue保持不变，也就是说当这三种类型的声音同时播放的时候，会降低Music和SFX的音量，以凸显Dialogue的声音。
## 用法
- 设置到SoundClass，这个是被动音效混合修改器
![](./img/8.png)
上图中是直接在SoundClass的被动混音修改器里面添加的混音配置，从名字可以知道这个效果是被动的，只要这个类型的音效被播放，就会自动触发这个混合效果。
- 蓝图
![](./img/9.png)
在蓝图里面，可以调用PushSoundMixModifer和PopSoundMixModifer来手动激活和停用某个音效混合修改器，这里就不是被动的了，因为我们手动控制了混合修改器的开始和结束时间。
注意:这里的mix是会累乘的，一定要注意!!!
# 衰减和空间化


# 源效果

# 总线

# 子混合和子混合发送

# 混响
## 音频体积
## 延迟
## 卷积

# 侧链压缩

# 优先级和并发

# 调试
