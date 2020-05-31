#ifndef PAGE_H
#define PAGE_H

#include <vector>
#include <memory>
#include <string>

#include "camera.h"
#include "game_object.h"
#include "sprite_renderer.h"
#include "text_renderer.h"
#include "page_ui.h"

class Page {
public:
    static constexpr float TILE_DIMENSIONS = 100.0f;
    static constexpr float TILE_SCALE_RATIO = 0.05;

	glm::vec2 Position, Size;

	bool Placing = false;

	Camera2D * Camera;
	std::vector<GameObject *> Pieces;
	Texture2D Board_Texture;
	SpriteRenderer * Renderer;
	PageUI * UserInterface;

	std::string Name;

	~Page();
	Page(std::string name, Texture2D board_tex, SpriteRenderer * renderer,
		 std::shared_ptr<std::pair<int, int>> screenDims,
		 glm::vec2 pos = glm::vec2(0.0f, 0.0f),
		 glm::vec2 size = glm::vec2(100.0f, 100.0f));

	// Mouse event handlers
	void HandleLeftClickPress(glm::ivec2 mouse_pos);
	void HandleLeftClickHold(glm::ivec2 mouse_pos);
	void HandleLeftClickRelease(glm::ivec2 mouse_pos);
	void HandleRightClick(glm::ivec2 mouse_pos);
	void HandleMiddleClickPress(glm::ivec2 mouse_pos);
	void HandleMiddleClickHold(glm::ivec2 mouse_pos);
	void HandleScrollWheel(int direction);

	// Place piece on board, optionally grid locked
	void PlacePiece(GameObject * piece, bool grid_locked = true);
	// Begin placing a piece on board, this locks it to the mouse and doesn't place until clicked.
	void BeginPlacePiece(GameObject * piece);

	void Update(float dt);
	// If placing a piece, call this
	void UpdatePlacing(glm::ivec2 mouse_pos);

	void Draw(SpriteRenderer * sprite_renderer, TextRenderer * text_renderer);

private:
	GameObject * CurrentSelection = nullptr;
	glm::ivec2 DragOrigin = glm::ivec2(0);
	std::shared_ptr<std::pair<int, int>> ScreenDims;

	void SnapPieceToGrid(GameObject * piece);
	glm::vec2 ScreenPosToWorldPos(glm::ivec2 pos);

	void HandleUIEvents();
};

#endif