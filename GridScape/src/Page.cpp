#include <iostream>

#include "page.h"
#include "resource_manager.h"
#include "text_renderer.h"
#include "text_object.h"

#include <glm/gtx/string_cast.hpp>

Page::Page(std::string name, Texture2D board_tex, SpriteRenderer * renderer,
		   std::shared_ptr<std::pair<int, int>> screenDims, glm::vec2 pos,
		   glm::vec2 size)
	: Name(name), Board_Texture(board_tex), Renderer(renderer),
	ScreenDims(screenDims), Position(pos), Size(size) {
	this->Renderer->Resize((int)this->Size.x);
	this->Camera = new Camera2D(200.0f,
								glm::vec2(this->Size.x * this->TILE_DIMENSIONS,
										  this->Size.y * this->TILE_DIMENSIONS),
								glm::vec2(0.4f, 2.5f),
								this->ScreenDims);
	this->UserInterface = new PageUI(this->ScreenDims);
}
Page::~Page() {
	for (GameObject * piece : this->Pieces) {
		delete piece;
	}
	delete this->Camera;
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
	this->Renderer->DrawSprite(this->Board_Texture, this->Position, false, this->Size * this->TILE_DIMENSIONS);
	for (GameObject * piece : this->Pieces) {
		piece->Draw(sprite_renderer, piece == this->CurrentSelection);
	}
	// If placing a piece, it's not part of the board yet, draw it seperately
	if (this->Placing)
		this->CurrentSelection->Draw(sprite_renderer, true);
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
			auto click_pos = piece->DistanceFromTopLeft(world_mouse);
			auto click_ratio = click_pos / piece->Size;
			// Check if the user is clicking on any of the sprite's edges /
			// corners
            piece->ScaleEdges = std::make_pair(0, 0);
			if (click_ratio.x <= TILE_SCALE_RATIO) {
                piece->ScaleMouse = true;
                piece->ScaleEdges.first = -1;
			}else if (1 - click_ratio.x <= TILE_SCALE_RATIO) {
                piece->ScaleMouse = true;
                piece->ScaleEdges.first = 1;
			}
            if (click_ratio.y <= TILE_SCALE_RATIO) {
                piece->ScaleMouse = true;
                piece->ScaleEdges.second = -1;
            }else if (1 - click_ratio.y <= TILE_SCALE_RATIO) {
                piece->ScaleMouse = true;
                piece->ScaleEdges.second = 1;
            }
            piece->initialSize = piece->Size;
            piece->initialPos = piece->Position;
            // If not scaling, enable dragging instead
            piece->FollowMouse = !piece->ScaleMouse;
            this->CurrentSelection = piece;
			this->DragOrigin = click_pos;
			break;
		}
	}
}

void Page::HandleLeftClickHold(glm::ivec2 mouse_pos) {
	glm::vec2 world_mouse = this->ScreenPosToWorldPos(mouse_pos);
	if (this->CurrentSelection) {
		if (this->CurrentSelection->FollowMouse) {
			this->CurrentSelection->Position = world_mouse - (glm::vec2)this->DragOrigin;
		} else if (this->CurrentSelection->ScaleMouse) {
		    auto diff = this->CurrentSelection->DistanceFromTopLeft(world_mouse) -
		            (glm::vec2)this->DragOrigin;
		    switch (this->CurrentSelection->ScaleEdges.first) {
		        case 1:
		            this->CurrentSelection->Size.x =
		                    this->CurrentSelection->initialSize.x + diff.x;
		            break;
		        case -1:
                    this->CurrentSelection->Position.x = world_mouse.x -
                            this->DragOrigin.x;
                    this->CurrentSelection->Size.x =
                            this->CurrentSelection->initialSize.x +
                            this->CurrentSelection->initialPos.x -
                            this->CurrentSelection->Position.x;
		            break;
		    }
		    switch(this->CurrentSelection->ScaleEdges.second) {
		        case 1:
                    this->CurrentSelection->Size.y =
                            this->CurrentSelection->initialSize.y + diff.y;
                    break;
                case -1:
                    this->CurrentSelection->Position.y = world_mouse.y -
                            this->DragOrigin.y;
                    this->CurrentSelection->Size.y =
                            this->CurrentSelection->initialSize.y +
                                    this->CurrentSelection->initialPos.y -
                                    this->CurrentSelection->Position.y;
                    break;
		    }
		}
	}
}

void Page::HandleLeftClickRelease(glm::ivec2 mouse_pos) {
	glm::vec2 world_mouse = this->ScreenPosToWorldPos(mouse_pos);
	if (this->CurrentSelection) {
		if (this->CurrentSelection->FollowMouse) {
		    // TODO: Re-enable snapping on release once a snap-modifier key
		    // is added
		    // this->SnapPieceToGrid(this->CurrentSelection);
		}
        this->CurrentSelection->FollowMouse = false;
		this->CurrentSelection->ScaleMouse = false;
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
