#pragma once

// ゴールまでの移動回数・固定する階層・部屋タイプの確率は FloorConfig(FloorEditor で編集)に移した
constexpr int kEarlyFloorNumber = 3; // この階層までは敵の出現テーブルが First.json

// 部屋の予定(RoomPlan)をファイルに出して編集する(デバッグ用)
constexpr bool kSaveFloorPlan = false; // true: 予定が決まるたびにファイルへ書く
constexpr bool kLoadFloorPlan = false; // true: フロア生成後にファイルの予定を読む
constexpr const char* kFloorPlanPath = "Resources/Data/Floor/FloorPlan.json";
