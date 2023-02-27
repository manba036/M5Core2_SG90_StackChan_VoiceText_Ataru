# M5Core2_SG90_StackChan_VoiceText_Ataru forked from @robo8080

[@robo8080さん](https://github.com/robo8080/M5Core2_SG90_StackChan_VoiceText_Ataru)からフォークし、[@katsuyoshiさん](https://github.com/katsuyoshi/M5Core2_SG90_StackChan_VoiceText_Ataru)のコードを参考にさせてもらったものです。

## ｽﾀｯｸﾁｬﾝを動かすための準備

- ./sdcard内のtxtファイル4つをSDカード直下にコピーし、適切に編集※し、M5Stack Core2に挿入してください  
  ※各txtファイル(以後、設定ファイル)の冒頭などに説明を記載しているので編集の参考にしてください
- `settings.txt`の`TTS_API_KEY`と`wifi_info.txt`の`YOUR_WIFI_SSID`/`YOUR_WIFI_PASSWORD`1組を記入してもらえれば、ｽﾀｯｸﾁｬﾝが喋るようになると思います  
  【補足】TTS_API_KEYについてはrobo8080さんのオリジナルのドキュメント(↓)を確認ください
- 一応「SDカードが挿入されていない場合」or「SDカードは挿入されているが所定の設定ファイルがない場合」でも最低限の動作は行えるようにデフォルト値を仕込んであります
- 一応、設定ファイルは互換性を持たせているので、設定ファイルそのままで、ファームウェアを0.2.0から0.2.1に更新しても問題なく動きますが、  
  0.2.1で時刻指定等を改善しているので、**0.2.1の./sdcardの設定ファイルを参考に設定ファイルを更新することをおススメします**

### このプログラムをビルドするのに必要な物(manba036版)

最新環境でビルドができない、動作しないといった場合まずはバージョンを下記に合わせてみてください。

* Arduino IDE (バージョン 1.8.19で動作確認をしました。)
* ボードマネージャ: M5Stack(2.0.5-1.0で動作確認しました。)
* [M5Unified](https://github.com/m5stack/M5Unified)ライブラリ(バージョン 0.0.7で動作確認をしました。)
* [M5Stack-Avatar](https://github.com/meganetaaan/m5stack-avatar/ "Title")ライブラリ(バージョン 0.8.2で動作確認をしました。)
* [ServoEasing](https://github.com/ArminJo/ServoEasing/ "Title")ライブラリ(バージョン 2.4.0で動作確認をしました。)
* [ESP32Servo](https://github.com/madhephaestus/ESP32Servo/ "Title")ライブラリ(バージョン 0.9.0で動作確認をしました。)
* [ESP8266Audio](https://github.com/earlephilhower/ESP8266Audio/ "Title")ライブラリ(バージョン 1.9.7で動作確認をしました。)

---
## 変更点(0.2.0→0.2.1)

1. 本家の変更( [サーボループでlipsyncが占有されるため、別タスク化](https://github.com/robo8080/M5Core2_SG90_StackChan_VoiceText_Ataru/pull/2) )を取り込み
1. スタックちゃんの水色化対策で[M5Stack-Avatar](https://github.com/meganetaaan/m5stack-avatar/)ライブラリのバージョンを0.8.1から0.8.2に変更
1. 時刻通知／時刻指定通知を4点改善  
  1点目) 時刻通知の喋りを「HH時0分」を「HH時ちょうど」、「HH時30分」を「HH時半」に変更  
  2点目) 時刻指定通知の通知時刻を「HH」と「MM」の2行から「HH:MM」の1行に変更  
  3点目) 時刻指定通知の通知時刻を「時刻＋曜日」に拡張  
  4点目) 時刻指定通知のメッセージ数を最大128個に変更
1. メッセージ数を最大64個に変更

## 変更点(本家→0.2.0)

1. 各種動作設定(TTS_API_KEY含む)、WiFi情報等をSDカード内の各種設定ファイルに移動しました(manba036オリジナル)  
  **基本、ファームウェアを再ビルド／アップロードすることなく、SDカード内の各種設定ファイルを変更するだけで、各種動作を変更することができます**
1. WiFiMultiで複数のWiFiポイントに接続できます(katsuyoshiさん追加機能を移植)  
  SDカード内の設定ファイルにWiFiのSSID/PASSWORDを複数登録できます(manba036オリジナル)
1. PandFaceとTVFaceを追加しました(katsuyoshiさん追加機能を移植)  
  Button A: __あたる__(0) と __スースー__(3) を交互に切り替えます(喋りません)  
  Button B: __ラム__(1) に切り替えます(喋りません)  
  Button C: __スタック__(2) と __ブラウン__(4) を交互に切り替えます(喋りません)  
  SDカード内の設定ファイルで起動時(デフォルト)のアバターを指定できます(manba036オリジナル)  
1. 顔(画面)をタップする毎に同じアバターで別のメッセージを喋ります(manba036オリジナル)  
  SDカード内の設定ファイルにアバターが喋るメッセージを複数登録できます  
  アバター毎に喋るメッセージを変えられるようにしました  
  各メッセージに感情を指定できるようにしました  
  メッセージの言い出しをランダムで変えるようにしました(プログラムで固定)  
  時刻に合わせて挨拶(おはよう／こんばんは等)を変えるようにしました(プログラムで固定)  
1. SDカード内の設定ファイルでサーボ用ポートを指定できます(manba036オリジナル)  
  **Port.C(X:G13, Y:G14)からPort.A(X:G33, Y:G32)に変更しています**  
  SDカード内の設定ファイルでサーボ調整値を指定できます  
1. 一定間隔で時刻通知するようにしました(katsuyoshiさん追加機能を移植)  
  夜間などは時刻通知を抑制できるようにしました(manba036オリジナル)  
  SDカード内の設定ファイルで「時刻通知の間隔」「時刻通知する時間帯」等を指定できます
1. 指定の時刻に時刻通知と合わせて指定のメッセージを喋るようにしました(manba036オリジナル)  
  SDカード内の設定ファイルで「指定の時刻」「指定のメッセージ」を複数登録できます

---
以下オリジナルのドキュメントです。

# M5Core2_SG90_StackChan_VoiceText_Ataru
M5Core2_SG90_StackChan_VoiceText_Ataru

@mongonta555 さんが[ｽﾀｯｸﾁｬﾝ M5GoBottom版組み立てキット](https://raspberrypi.mongonta.com/about-products-stackchan-m5gobottom-version/ "Title")の頒布を始められたので、それに対応したｽﾀｯｸﾁｬﾝファームを作ってみました。<br>

![画像ataru](images/ataru.png)<br>

---
### M5GoBottom版ｽﾀｯｸﾁｬﾝ本体を作るのに必要な物、及び作り方 ###
こちらを参照してください。<br>
* [ｽﾀｯｸﾁｬﾝ M5GoBottom版組み立てキット](https://raspberrypi.mongonta.com/about-products-stackchan-m5gobottom-version/ "Title")<br>

### このプログラムをビルドするのに必要な物 ###

※ 2022/9/24の変更でM5Unifiedに対応しました。
最新環境でビルドができない、動作しないといった場合まずはバージョンを下記に合わせてみてください。

* arduino-esp32(v2.0.5で動作確認しました。)
* Arduino IDE (バージョン 1.8.16で動作確認をしました。)<br>
* [M5Unified](https://github.com/m5stack/M5Unified)ライブラリ(バージョン 0.0.7で動作確認をしました。)
* [M5Stack-Avatar](https://github.com/meganetaaan/m5stack-avatar/ "Title")ライブラリ(バージョン 0.8.1で動作確認をしました。)<br>※ v0.8.1だと標準のAvatarFaceホワイトが水色になってしまう不具合がありますが動作に支障はありません。
* [ServoEasing](https://github.com/ArminJo/ServoEasing/ "Title")ライブラリ(バージョン 2.4.0で動作確認をしました。)<br>
* [ESP32Servo](https://github.com/madhephaestus/ESP32Servo/ "Title")ライブラリ(バージョン 0.9.0で動作確認をしました。)<br>
* [ESP8266Audio](https://github.com/earlephilhower/ESP8266Audio/ "Title")ライブラリ(バージョン 1.9.7で動作確認をしました。)<br><br>

---

---

M5Stack Core2の場合、M5Stack-AvatarとHOYA社が提供する[VoiceText Web APIサービス](https://cloud.voicetext.jp/webapi "Title")を使った音声合成(TTS)を使うことができます。


VoiceText TTSは、kghrlaboさんのesp32_text_to_speechを参考にさせていただきました。<br>
オリジナルはこちら。<br>
esp32_text_to_speech <https://github.com/kghrlabo/esp32_text_to_speech><br>

### VoiceTextの有効化 ###
* M5Core2_SG90_StackChan_VoiceText_Ataru.inoの22行目のコメントを外して”USE_VOICE_TEXT”を有効化してください。

### WiFiの設定 ###
* M5Core2_SG90_StackChan_VoiceText_Ataru.inoの31行目付近、SSIDとPASSWORDを設定してください。

### VoiceText Wev API api キーの設定 ###
* AudioFileSourceVoiceTextStream.cppの30行目付近、YOUR_TSS_API_KEYを設定してください。<br>
APIキーは、[ここ](https://cloud.voicetext.jp/webapi/ "Title")の「無料利用登録」から申請すれば、メールで送られて来ます。<br>

---

### 使い方 ###
* VoiceTextを有効にしていない場合：M5Stack Core2のボタンA,B,Cを押すと、それぞれ異なった顔を表示します。　<br>
* VoiceTextを有効にしている場合：M5Stack Core2のボタンA,B,Cを押すと、それぞれ異なった顔と声でしゃべります。　<br>
音声データをダウンロード中は顔にハートマークが表示されます。<br>
TTSのパラメータの詳細はこちらを参照してください。<br>
[VoiceText Web API [API マニュアル](https://cloud.voicetext.jp/webapi/docs/api/ "Title")]
<br><br>

