# ファイル管理補足

## 参照カウントの悪用例

ここで紹介するのは、Linuxの話で、「ip netns」コマンドについてである。「ip netns add <名前>」で新しいネットワークネームスペースを作成できる。ネットワーク関連の動作確認とかテストに非常に重宝するのので、筆者もよく使っている。

始めてこれを見たときには、あれ、どうして？と不思議に思った。多くのユーザは何の疑問もなく使用しているのだろうし、きちんと実装を理解している人は何も不思議に思わないわけで、不思議に思うのは中途半端に理解している人だけ(すなわち、筆者)なのだろう。どうして不思議に思ったかと言うと、ネームスペースというものは、基本的に親プロセスから引き継ぐもので、変えられるのは、clone(clone3)実行時か、unshare(2)実行時である。「ip netns add」では、unshare(2)で新しいネットワークネームスペースを作っているだろうことまでは分かるが、それを使えるのは、子プロセスか、子プロセス以外でも使えなくはないが、それは、ipコマンドが生きている間だけのはずである。ipコマンドは死んでしまうのに、その後に使えてしまうのはどうした訳だろう、ということだ。

謎解きをすると、まず、筆者の知らない間に、procファイルシステムの下に /proc/{pid}/ns/ というディレクトリができていた。その配下には各種ネームスペース用ファイルが存在する。ネットワークネームスペース用は、/proc/{pid}/ns/net である。これはオープンしてみても、何かが出来るわけではないが、オープン(またはそれに類する操作)をすると、それ用のinodeが作成され、そのinodeからネームスペース用の制御表がポイントされて、その(inodeおよび)ネームスペース用制御表の参照カウントが上がるというところがポイントなのである。

「ip netns add <名前>」を実行すると、以下の処理が実行される。

- (1) unshareで新しいネットワークネームスペースを作成。
- (2) /run/netns(大抵は、tempfs)の下に<名前>ファイルを作成する。(通常ファイル)
- (3) /proc/self/ns/net を /run/netns/<名前>にマウントする。(補足: /proc/self は /proc/{自身のpid} と同じ)

(3)の操作により、(inodeおよび)ネームスペース用制御表の参照カウントがさらにアップする。そのため、ip コマンドが終了してしまっても、(inodeおよび)ネームスペース用制御表は、削除されずに生き残るという訳だ。しかも、/run/netns/<名前> をオープンすれば、ネームスペース用制御表にアクセスできるので、setnsなどで利用できるという訳だ。

こうして今では不思議ではなくなった。それにしても、昔は、マウントと言えば、ディレクトリの上にファイルシステム(のルートディレクトリ)をマウントできるだけだったのだが、今では、ファイルの上にファイルをマウントしたり、何でもありだ。本件書くためにmanページ等確認していたら、pidfd_open(2)なるものまで見つけてしまった。これも、ファイルじゃないのにファイルディスクリプタを使う例のひとつだね。もう本当になんでもありだな。