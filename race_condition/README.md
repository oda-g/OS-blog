# 競合状態

## カウント

カウントのインクリメントで競合が起きる例を用意した。プログラムはこちら([count_ng.c](https://github.com/oda-g/OS-blog/tree/main/race_condition/count_ng.c))。

親子のプロセスが共有メモリ上のカウント(total_count)を100000000回ずつインクリメントしている。競合が起きなければ、プログラム終了時に期待される結果は、200000000のはずである。

このプログラムでは、インクリメント部分は、以下のようになっており、何の対処もしていない。(親の部分を抜粋)
```
                for (i = 0; i < NUM_COUNT; i++) {
                        (*total_count)++;
                }
```

プログラムを実際に動かした結果は以下のとおり。命令レベルで競合が起きることを実感するために、nativeの環境で行って見る。
```
$ gcc -o count_ng count_ng.c
$ sudo taskset -a 1 ./count_ng
total_count: 156372645
```

CPUがひとつでも競合が起きることを確認するために、tasksetで実行するCPUを指定している。上記例では、親子プロセスとも、CPU 0 で実行される。

補足:
- MP環境では、tasksetを指定しないと、親子プロセスが別のCPUで同時に動くため、もっとNUM_COUNTが少なくても、競合を再現できる。UP環境においては、タイムスライスによるプロセス切り替えが十分な数だけ起きるくらいの時間を要するNUM_COUNT数にする必要がある。
- 起きるかどうか、起きたときのtotal_countは環境により異なる。  
本プログラムは延々とカウントアップしているだけで、そこで実行されている命令数は数命令程度である。そのため、まずい箇所で割り込まれる確率も結構高い(数分の１の確率)と言える。  
結果の数が大きく違っているのは、タイムスライス時間の間カウントアップした数が巻き戻るためである。 
- RISC-Vバイナリを作って、qemuで実行した場合でも再現する。

セマフォにより、クリティカルセクションを保護した例がこちら([count_ok.c](https://github.com/oda-g/OS-blog/tree/main/race_condition/count_ok.c))。プログラムは以下のようになっている。
```
                for (i = 0; i < NUM_COUNT; i++) {
                        if (sem_wait(sem) == -1) {
                                perror("parent sem_wait");
                                err = 1;
                                break;
                        }
                        (*total_count)++;
                        (void)sem_post(sem); /* there is no error case */
```

プログラムを実行すると期待した値となる。
```
$ gcc -o count_ng count_ok.c -pthread
$ sudo taskset -a 1 ./count_ok
total_count: 200000000
```

補足:
- sem_wait/sem_postのオーバヘッドのため、結構遅くなる。  
なお、Linuxでのsem_wait/sem_postの実装には、futexシステムコールが使用されている。競合の判定に関しては、システムコールを発行することなく行えるような工夫がされている。とは言え、オーバヘッドは結構ある。
- 本対策は、MP環境でも有効。tasksetを指定せず、別CPUで並列実行しても期待した値となる。

実は、カウントのインクリメントに関しては、ロックを使用しなくても競合状態を防ぐことができる。大体どんなアーキテクチャにでも備わっているアトミック命令を使用する。プログラム例はこちら([count_at.c](https://github.com/oda-g/OS-blog/tree/main/race_condition/count_at.c))。
```
                for (i = 0; i < NUM_COUNT; i++) {
                        __atomic_fetch_add(total_count, 1, __ATOMIC_RELAXED);
                }
```

実行例は割愛。__atomic_fetch_add というのは、コンパイラが用意している関数で、アーキテクチャに応じた命令に落としてくれる。RISC-V の場合(*)、以下の命令が使用されていた。

(*)「zig cc -target riscv64-linux-musl count_at.c -o rv_count_at」でバイナリ作成

```
   ld      a0, 0(a0)             a0 = &total_count
   li      a1, 1                 a1 = 1
   sw      a1, -32(s0)           --+ a1の値をスタックに退避して、ロード。この部分は、謎。
   lw      a1, -32(s0)           --+
   amoadd.w        a0, a1, (a0)  「*a0 += a1」を一命令で実行
```

ポイントは「amoadd.w」で、1命令でメモリの値を更新している。1命令で実行しているので、競合は起きないのである。

補足:
- gccでx86_64 nativeバイナリを作った場合については、割愛。動作はさせているが、バイナリまで調べていない。
- 本対策は、MP環境でも有効。tasksetを指定せず、別CPUで並列実行しても期待した値となる。  
これに限らず、アトミック命令は、MP環境での競合防止にいろいろと使われている。そこらへんの話は、MPの話題のときに説明するかもしれない。

## リンクリスト

リンクリストの操作は、クリティカルセクションであり、競合を防ぐ必要があるというのは、当たり前と言えば当たり前なのであるが、一応、具体的に見ておこう。ここでは、最も単純の片キューリストで、LIFO操作をするケースを考える。

```
              A              B
  head --> +------+  +--> +------+  +--> null
           | next |--+    | next |--+
           |      |       |      |
           +------+       +------+
```
headに繋ぐ操作: (Cを繋ぐとする)
```
(1)  C.next = head
(2)  head = &C
```
headから外す操作: (外したものを返すとする)
```
(3)  ret = head
     if (ret != NULL) {
(3)     head = ret->next
        ret->next = NULL
     }
     return ret
```
上記の2つの処理が実行されたとして、完了したとき、正しい状態は、以下のどちらかとなる。
- head --> A --> B  (繋ぐ方が先だった。Cを繋ぎ、Cを外す)
- head --> C --> B  (外す方が先だった。Aを外し、Cを繋ぐ)

処理が競合して、2つのスレッドが以下の時系列で処理が実行されたとする。
- スレッドX (1)
- スレッドY (3)
- スレッドX (2)

そうすると、最終的な状態は、以下のようになり、大変まずい。
- head --> C --> A  
(スレッドY: return A)

上記は、繋ぐ操作と外す操作の競合時であるが、繋ぐ操作と繋ぐ操作、外す操作と外す操作も競合状態が発生する。繋ぐ操作と外す操作の部分をロック等で排他する必要がある。

