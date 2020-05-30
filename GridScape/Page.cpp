#include <iostream>

#include "page.h"
#include "resource_manager.h"
#include "text_renderer.h"
#include "text_object.h"

#include <glm/gtx/string_cast.hpp>

Page::Page(std::string name, Texture2D board_tex, SpriteRenderer * renderer,
		   unsigned int width, unsigned int height, glm::vec2 pos, glm::vec2 size)
	: Name(name), Board_Texture(board_tex), Renderer(renderer),
	WindowWidth(width), WindowHeight(height), Position(pos), Size(size) {
	this->Renderer->Resize((int)this->Size.x);
	this->Camera = new Camera2D(200.0f,
								glm::vec2(this->Size.x * this->TILE_DIMENSIONS,
										  this->Size.y * this->TILE_DIMENSIONS),
								glm::vec2(0.4f, 2.5f));
	this->Camera->ScreenDims = glm::vec2(this->WindowWidth, this->WindowHeight);
	this->SelectionBox = new GameObject(glm::vec2(0.0f), glm::vec2(0.0f),
										ResourceManager::GetTexture("selection"), false);
	this->UserInterface = new PageUI(this->WindowWidth, this->WindowHeight);
}
Page::~Page() {
	for (GameObject * piece : this->Pieces) {
		delete piece;
	}
	delete this->Camera;
	delete this->SelectionBox;
}

void Page::PlacePiece(GameObject * piece, bool grid_locked) {
	if (grid_locked)
		piece->Position = glm::vec2(piece->Position.x * this->TILE_DIMENSIONS + 1,
									piece->Position.y * this->TILE_DIMENSIONS + 1);
	this->Pieces.push_back(piece);
}

void Page::BeginPlacePiece(GameObject * piece) {
	this->CurrentSelection = piece;
	this->Placing = true;
}

void Page::Update(float dt) {
	this->Camera->Update(dt, this->Size * this->TILE_DIMENSIONS);
	this->HandleUIEvents();
}

void Page::UpdatePlacing(glm::ivec2 mouse_pos) {
	glm::vec2 world_mouse = this->ScreenPosToWorldPos(mouse_pos);
	this->CurrentSelection->Position = world_mouse - this->CurrentSelection->Size / 2.0f;
}

void Page::Draw(SpriteRenderer * sprite_renderer, TextRenderer * text_renderer) {
	this->Renderer->View = this->Camera->View;
	sprite_renderer->View = this->Camera->View;
	this->Renderer->DrawSprite(this->Board_Texture, this->Position, this->Size * this->TILE_DIMENSIONS);
	for (GameObject * piece : this->Pieces) {
		piece->Draw(sprite_renderer);
	}
	// If placing a piece, it's not part of the board yet, draw it seperately
	if (this->Placing)
		this->CurrentSelection->Draw(sprite_renderer);
	// Draw selection box if something is selected
	if (this->CurrentSelection) {
		this->SelectionBox->Position = this->CurrentSelection->Position;
		this->SelectionBox->Size = this->CurrentSelection->Size;
		this->SelectionBox->Draw(sprite_renderer);
	}
	// Draw user interface
	this->UserInterface->DrawPieceClickMenu();
}

void Page::HandleUIEvents() {
	if (this->UserInterface->MoveToFront) {
		for (int i = 0; i < this->Pieces.size(); i++) {
			if (this->Pieces[i] == this->CurrentSelection)
				this->Pieces.erase(this->Pieces.begin() + i);
		}
		this->Pieces.push_back(this->CurrentSelection);
	} else if (this->UserInterface->MoveToBack) {
		for (int i = 0; i < this->Pieces.size(); i++) {
			if (this->Pieces[i] == this->CurrentSelection)
				this->Pieces.erase(this->Pieces.begin() + i);
		}
		this->Pieces.insert(this->Pieces.begin(), this->CurrentSelection);
	}
	this->UserInterface->ClearFlags();
}

void Page::HandleLeftClickPress(glm::ivec2 mouse_pos) {
	glm::vec2 world_mouse = this->ScreenPosToWorldPos(mouse_pos);
	// Piece that is currently being placed
	if (this->Placing) {
		this->SnapPieceToGrid(this->CurrentSelection);
		this->PlacePiece(this->CurrentSelection, false);
		this->Placing = false;
		return;
	}
	this->CurrentSelection = nullptr;
	for (GameObject * piece : this->Pieces) {
		if (piece->CheckContainment(world_mouse) && piece->Clickable) {
			this->CurrentSelection = piece;
			this->CurrentSelection->FollowMouse = true;
			this->DragOrigin = this->CurrentSelection->DistanceFromTopLeft(world_mouse);
		}
	}
}

void Page::HandleLeftClickHold(glm::ivec2 mouse_pos) {
	glm::vec2 world_mouse = this->ScreenPosToWorldPos(mouse_pos);
	if (this->CurrentSelection) {
		if (this->CurrentSelection->FollowMouse) {
			this->CurrentSelection->Position = world_mouse - (glm::vec2)this->DragOrigin;
		}
	}
}

void Page::HandleLeftClickRelease(glm::ivec2 mouse_pos) {
	glm::vec2 world_mouse = this->ScreenPosToWorldPos(mouse_pos);
	if (this->CurrentSelection) {
		if (this->CurrentSelection->FollowMouse) {
			this->SnapPieceToGrid(this->CurrentSelection);
			this->CurrentSelection->FollowMouse = false;
		}
	}
}

void Page::HandleRightClick(glm::ivec2 mouse_pos) {
	if (this->CurrentSelection)
		this->UserInterface->ClickMenuActive = true;
}

void Page::HandleMiddleClickPress(glm::ivec2 mouse_pos) {
	this->DragOrigin = mouse_pos;
}

void Page::HandleMiddleClickHold(glm::ivec2 mouse_pos) {
	glm::vec2 v = mouse_pos - DragOrigin;
	this->Camera->Move(v);
	this->DragOrigin = mouse_pos;
}

void Page::HandleScrollWheel(int scroll_direction) {
	if (this->CurrentSelection) {
		if (scroll_direction == 1)
			this->CurrentSelection->Size += 5;
		else if (scroll_direction == -1)
			this->CurrentSelection->Size -= 5;
	} else {
		if (scroll_direction == 1)
			this->Camera->ZoomIn();
		else if (scroll_direction == -1)
			this->Camera->ZoomOut();
	}
}

void Page::SnapPieceToGrid(GameObject * piece) {
	float closest_x = (float)floor(piece->Position.x / this->TILE_DIMENSIONS + 0.5);
	float closest_y = (float)floor(piece->Position.y / this->TILE_DIMENSIONS + 0.5);
	if (closest_x > this->Size.x)
		closest_x = this->Size.x - 1;
	if (closest_x < 0)
		closest_x = 0;
	if (closest_y > this->Size.y)
		closest_x = this->Size.y - 1;
	if (closest_y < 0)
		closest_y = 0;
	piece->Position = glm::vec2(closest_x * this->TILE_DIMENSIONS + 1,
								closest_y * this->TILE_DIMENSIONS + 1);
}

glm::vec2 Page::ScreenPosToWorldPos(glm::ivec2 pos) {
	return (glm::vec2)pos / this->Camera->Zoom + this->Camera->Position;
}
