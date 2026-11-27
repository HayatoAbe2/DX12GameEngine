#pragma once

/// <summary>
/// キャラが再生するアニメーション1つ分の設定
/// </summary>
struct CharacterAnimation {
	int index = -1;          // モデル内のアニメーション番号(-1で使わない)
	float duration = -1.0f;  // 全体の再生時間(秒)。0以下なら元の長さのまま
};
