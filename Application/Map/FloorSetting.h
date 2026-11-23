#pragma once

constexpr int kEarlyFloorNumber = 3; // 序盤扱いの部屋

constexpr bool kSaveFloorPlan = false; // true: 予定が決まるたびにファイルへ書く
constexpr bool kLoadFloorPlan = false; // true: フロア生成後にファイルの予定を読む
constexpr const char* kFloorPlanPath = "Resources/Data/Floor/FloorPlan.json";
