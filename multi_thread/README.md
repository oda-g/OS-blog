# マルチスレッド

## Linuxのデータ構造

Linuxでは、プロセスを管理する制御表は、task_struct 構造体である、というのはシングルスレッド時代は、正にその通りであったが、現在は、task_struct は、スレッドを管理する制御表であると言った方が正しい。すなわち、ひとつのスレッドを実行するために必要な情報を全て管理しているもの、である。Linuxでのプロセスとは、task_struct のグループであるという捉え方になっている。

Linuxでは、スレッドを複製するシステムコール clone、clone3 が用意されている。clone3 は、比較的新しく、clone のスーパーセットになっている。clone(2)のman ページを見ると、glibc の clone wrapper の説明が書いてあって、まぎらわしいが、ここでの clone は、本当のシステムコールのことである。clone は、fork を代替可能であり、現在、glibc では、fork は、clone を使用している(fork(2)ではなく、fork(3)になってしまっている)。strace、ebpf等使ったときにびっくりしないように。なお、fork自体がなくなったわけではない。どうしても使いたい場合は、syscallを使用する。clone wrapperは、所謂スレッドの生成に使用できるものであるが、pthread_createの実装では、clone3 を使用している。

clone、clone3 は、forkに比べて、非常に高機能であり、プロセス属性のどの部分を共有するかをフラグで細かく指定できるようになっている。すべてのフラグを説明することはしないが、主要なケースについて紹介する。

### fork

clone で、CLONE_CHILD_CLEARTID|CLONE_CHILD_SETTID|SIGCHLD フラグ指定で生成する。CLONE_CHILD_CLEARTID、CLONE_CHILD_SETTID については、説明を割愛する。SIGCHILDは、フラグではなく、スレッドの終了時に親に送信するシグナルを指定している(cloneではそんなものまで指定できる。SIGCHILDというのは、通常のプロセスの場合と同じ)。要は、特にフラグを指定せず、デフォルトの指定ということであり、デフォルトの場合、fork と同等になるようになっているということである。

task_structおよび、そこからポイントされている各種制御表を1セット別に用意し、丸々コピーすることになる。

![fork時のtask_struct](https://github.com/oda-g/OS-blog/blob/main/blog-diag/task_struct_fork.png)

(注意：丸々コピーといっても、tid など、変更されるものも当然ある。)

仮想空間は、ページテーブル１セットが別に用意され(したがって、仮想空間としては別)、ページのレベルで共有される。本来は内容丸々コピーなのであるが、リードオンリー領域に関してはページ共有、ライト可能領域に関しては、コピーオンライトで最初は共有、としているのは、別に説明したとおり。

### pthread_create

clone3で、CLONE_VM|CLONE_FS|CLONE_FILES|CLONE_SIGHAND|CLONE_THREAD|CLONE_SYSVSEM|CLONE_SETTLS|CLONE_PARENT_SETTID|CLONE_CHILD_CLEARTID フラグ指定で生成する。(スレッド終了時、シグナルは送信しない設定）

task_structはコピーされるが、ポイントされている制御表のうち、いくつかがコピーではなく、共用されることになる。どれを共有するかをフラグで指定しているわけである。

- CLONE_VM  
仮想空間(mm_struct)を共有。
- CLONE_FS  
filesystem関連の属性(root dir、current dir、umask)を共有。
- CLONE_FILES  
ファイルディスクリプタテーブルを共有。
- CLONE_SIGHAND  
シグナルハンドラを共有。
- CLONE_SYSVSEM  
SYSTEM V セマフォを共有。

![スレッド生成時のtask_struct](https://github.com/oda-g/OS-blog/blob/main/blog-diag/task_struct_clone.png)

(注意: task_structはコピーと言っても、tidなど、変更されるものも当然ある。)

(図では、CLONE_VM、CLONE_FILESの部分のみ図示。)

仮想空間は、完全に同じものとなる。

CLONE_THREAD というのが、マルチスレッドなプロセスを構成するためのフラグであり、これが指定されると、親がスレッドリーダーとなるグループが形成される。task_thread(すなわち各スレッド)には、tid (thread id)というユニークなidが付いている。tidを取得する gettid というシステムコールが存在している。それでは、pid (process id)はどうなっているのかと言うと、所謂シングルスレッドプロセス(正確には、CLONE_THREADなしで生成されたスレッド)では、tid == pid である。すなわち、getpid の結果 == gettid の結果となる。CLONE_THREAD指定で生成されたスレッドから、getpidシステムコールを発行すると、スレッドリーダーのtidが返るようになっている。また、getppidを発行すると、スレッドリーダのppid(スレッドリーダの親のpid)が返る。

なお、CLONE_THREAD指定時は、CLONE_VM、CLONE_SIGHAND は必須である。

CLONE_SETTLS はスレッドローカルストレージ関連(説明は割愛)、CLONE_PARENT_SETTIDとCLONE_CHILD_CLEARTIDは説明を割愛する。

なお、clone3は、forkと同様、子スレッドは、clone3復帰時点からの実行となる。pthread_createで指定した関数の実行は、ライブラリ側がお膳立てをして実行することになる。(関数終了時に pthread_exitが実行されるようにするなどのお膳立てをする。)

### 補足(データ構造の違い)

図示はしなかったが、task_struct には、実行コンテキストを管理するための thread_info 構造体がポイントされている。task_struct と thread_info は1対1の関係にある。ブログ本編のプロセスデータ構造の図は、task_structから複数のthread_info をポイントするというアプローチを取ったものと考えてよい。

Linuxでは、(実行コンテキスト自体は、thread_infoであるが)task_structをCPUのスケジュール単位と捉えており、それを主体にマルチスレッドのための拡張を行ったものと考えられる。

Linuxの取ったアプローチにより、共有される部分をきめ細かく制御することが可能となっている。上記、pthread_createの例で指定したフラグは、POSIXスレッドの仕様を満たすための指定であったが、CLONE_FSやCLONE_FILESを指定しないでスレッドを生成することも可能である。どの程度意味があるのか分からないが、いろいろな組み合わせが出来る。指定できるフラグは、30近くあり、資源の共有だけでなく、名前空間に関するものもあり、コンテナ管理ソフトから使用されているものと想定される。それにしても数多すぎで、オーバスペック感が否めない。clone(clone3)のコード見るのは大変そうである。

マルチスレッドプログラムに関しては、forkしたときにどうなるのかという疑問があろうかと思う。Linuxにおいては、デフォルトフラグ(CLONE_THREADはなし)で、呼び出し元のtask_structをコピーするだけのことで、ある意味分かり易い。pid、ppidの扱いにのみ注意が必要(というか分かり難い)で、スレッドからforkされた子プロセスのppidは、呼び出し元スレッドのスレッドグループリーダの親のpidとなる。(例を図示)

```
parent of main (tid: 100, pid:100)
  |
  | fork
  v
main (tid: 101, pid: 101, ppid: 100)  スレッドリーダ
  |
  | clone(CLONE_THREAD)
  v
thread (tid: 102, pid: 101, ppid: 100)
  |
  | fork
  v
child of thread (tid: 103, pid: 103, ppid: 100) スレッドリーダ
```

/proc の下はどうなるかというと、「ls /proc」(すなわち、/proc を readdir)すると、スレッドリーダのものしか出てこない。上の例だと、103は出てくるが、102は出てこない。ただし、「/proc/102」とフルパスで指定してアクセス可能である。このあたりの仕様も分かりづらい。

ブログ本編のデータ構造を採用した場合、fork したときの動作をどうするかであるが、子プロセスの実行コンテキスト管理データとしては、forkを呼び出したスレッドのものひとつだけを持った状態でスタートするのが適切であると考えられる。

### 補足(ファイル共有)

ファイルがどう共有されているのか、制御表の共有図をイメージすると分かり易い。

![同一ファイルを別プロセスがオープン](https://github.com/oda-g/OS-blog/blob/main/blog-diag/file_share_1.png)

同じファイルなので、inodeはひとつで共有。file構造体、ファイルディスクリプタテーブルは別々なので、一方のプロセスがcloseしても他方ではopenされたままだし、一方のプロセスがread/writeしても、他方のプロセスのファイルオフセットに変化はない。

![forkした場合の親と子](https://github.com/oda-g/OS-blog/blob/main/blog-diag/file_share_2.png)

ファイルディスクリプタテーブルは別であるが、file構造体は共有した形となる。file構造体を共有しているということは、read/write時のファイルオフセットを共有しているということ。ファイルディスクリプタテーブルは別なので、一方井のプロセスがcloseしても、他方のプロセスからは依然アクセス可能(file構造体の参照カウントがデクリメントされている)。また、一方のプロセスが新たにファイルをopenしても、他方からはアクセスできない。

![スレッド(CLONE_FILES)での共有](https://github.com/oda-g/OS-blog/blob/main/blog-diag/file_share_3.png)

フィルディスクリプタテーブルから共有している。どれかのスレッドがcloseすれば、全スレッドから見えなくなるし、どれかのスレッドがopenすれば、全スレッドからアクセス可能。