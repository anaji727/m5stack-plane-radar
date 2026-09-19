# 検証結果（2026-09-19）

## 実施済み

- 指定の dump1090-mutability の `aircraft.json` と `receiver.json` にHTTPで接続成功。
- 実機サーバーの形式は `aircraft` 配列、`now`、`altitude`、`speed`、`seen_pos`。`receiver.json` に緯度経度なし。
- 取得サンプル：4,950 bytes、29機。うち位置あり15機、位置鮮度15秒以内かつ地上機以外11機をパーサーが抽出。範囲フィルター適用前の件数。
- `pio run -e basic` 成功。ESP32 Arduino 2.0.14、espressif32 6.5.0、依存バージョンは platformio.ini に固定。
- 静的RAM 61,272 / 327,680 bytes（18.7%）。実行中のWi-Fi、JSON、画面バッファ等のヒープ使用量はこの数字に含まれない。
- アプリサイズ 1,200,921 / 1,310,720 bytes（91.6%）。4 MBパーティション構成内に収まる。
- 生成された `firmware.bin` は 1,207,216 bytes。
- 同一C++パーサーをPCでコンパイルし、実データと境界条件のテストに成功。
- テスト対象：mutability / FA形式、高度・速度の旧新フィールド、コールサインの空白除去、hexへのフォールバック、位置なし、範囲外座標、数値文字列、古い位置、地上機、方向不明、経度180度跨ぎ、取得後の経過時間を加えた失効、millisの周回。

ファームウェア側の最終ビルドで依存するArduino ESP32のUART実装に戻り値警告が1件あります。今回のアプリ側のコンパイルエラーはありません。

## 実機が必要な確認

- LCDの色・向き・文字の見やすさ、A/B/Cボタン、輝度切り替え。
- 実際の初回Wi-Fiポータル操作、設定保存と再起動。
- M5StackからのHTTP取得、ルーター再起動後の復帰、サーバー停止時の表示。
- 長時間稼働時のヒープ・断片化・給電の安定性。

ホストPCでのサーバー接続成功は、M5Stack実機からの接続や実画面での動作保証ではありません。
この配布では実機への書き込みを行っていません。

## テストを再実行する

PlatformIOで依存を取得した後、一般的なC++17コンパイラを使えます。

```sh
g++ -std=c++17 -I include -I .pio/libdeps/basic/ArduinoJson/src tests/parser_test.cpp -o parser-test
./parser-test
# 保存した実データを追加検証する場合
./parser-test /path/to/aircraft.json
```

今回のWindowsホストでは Zig 0.14.1 の `zig c++` で実行しました。
実際に取得した機体JSONと受信機JSONは配布物に含めていません。
