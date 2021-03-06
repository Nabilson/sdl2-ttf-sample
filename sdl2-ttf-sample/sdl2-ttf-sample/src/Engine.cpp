#include "Engine.h"
#include <exception>
#include<string>

using namespace std;

Engine::Engine() {
	this->window = NULL;
	this->renderer = NULL;
	this->cost_font = NULL;
	this->event = SDL_Event();

	std::cout << "Successfully initialized engine." << std::endl;
}

Engine::Engine(int width, int height) {
	this->WIN_W = width;
	this->WIN_H = height;
	this->window = NULL;
	this->renderer = NULL;
	this->cost_font = NULL;
	this->event = SDL_Event();

	std::cout << "Successfully initialized engine." << std::endl;
}

Engine::Engine(const char* title, int width, int height) {
	this->TITLE = title;
	this->WIN_W = width;
	this->WIN_H = height;
	this->window = NULL;
	this->renderer = NULL;
	this->cost_font = NULL;
	this->event = SDL_Event();

	std::cout << "Successfully initialized engine." << std::endl;
}

Engine::~Engine() {
	std::cout << "Successfully closed engine." << std::endl;
}

int Engine::load() {
	if (SDL_Init(SDL_INIT_VIDEO) != -1) {
		std::cout << "Successfully initialized SDL window." << std::endl;
	}
	else {
		std::cout << "Could not initialize SDL Video. Error: " << SDL_GetError() << std::endl;
		return 0;
	}

	this->window = SDL_CreateWindow(this->TITLE, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, this->WIN_W, this->WIN_H, SDL_WINDOW_SHOWN);

	if (this->window == NULL) {
		std::cout << "Could not initialize SDL Window. Error: " << SDL_GetError() << std::endl;
		return 0;
	}

	this->renderer = SDL_CreateRenderer(this->window, -1, SDL_RENDERER_ACCELERATED);

	if (this->renderer == NULL) {
		std::cout << "Could not initialize SDL Renderer. Error: " << SDL_GetError() << std::endl;
		SDL_DestroyWindow(this->window);
		return 0;
	}

	if (TTF_Init() == -1) {
		std::cout << "Could not initialize fonts. Error: " << SDL_GetError() << std::endl;
		SDL_DestroyWindow(this->window);
		SDL_DestroyRenderer(this->renderer);
		return 0;
	}

	std::cout << "Successfully initialized Engine." << std::endl;

	return 1;
}

int Engine::close() {
	try {
		TTF_Quit();
		SDL_DestroyWindow(this->window);
		SDL_DestroyRenderer(this->renderer);
		SDL_Quit();
		std::cout << "Successfully closed SDL." << std::endl;
		return 1;
	}
	catch (std::exception& e) {
		std::cout << "Could not close SDL. Error: " << e.what() << std::endl;
		return 0;
	}
}

void Engine::clearWindow() {
	SDL_SetRenderDrawColor(this->renderer, 0xFF, 0xFF, 0xFF, 0xFF);
	SDL_RenderClear(this->renderer);
}

void Engine::updateRenderer() {
	SDL_RenderPresent(this->renderer);
}

void Engine::drawNode(Node* node) {
	SDL_SetRenderDrawColor(this->renderer, node->red, node->green, node->blue, node->alpha);

	SDL_Rect rect;
	rect.x = node->x;
	rect.y = node->y;
	rect.w = node->width;
	rect.h = node->height;

	SDL_RenderFillRect(this->renderer, &rect);

	SDL_SetRenderDrawColor(this->renderer, 0x00, 0x00, 0x00, 0x00);
	SDL_RenderDrawRect(this->renderer, &rect);

	if (this->cost_mode && node->cost > 1) {

		SDL_Color color = { 0, 0, 0, 255 };

		std::string str = std::to_string(node->cost);

		SDL_Surface* surface = TTF_RenderText_Solid(this->cost_font, str.c_str(), color);

		SDL_Texture* txt = SDL_CreateTextureFromSurface(this->renderer, surface);

		int w;
		int h;

		SDL_QueryTexture(txt, NULL, NULL, &w, &h);

		SDL_Rect txtRect;

		if (node->cost >= 10) {
			txtRect = { node->x + (w / 6), node->y + (h / 8), w, h};
		}
		else if (node->cost < 10) {
			txtRect = { node->x + (int)(w / 1.25), node->y + (h / 8), w, h };
		}

		SDL_RenderCopy(this->renderer, txt, NULL, &txtRect);

		SDL_DestroyTexture(txt);
		SDL_FreeSurface(surface);
	}
}

void Engine::updateGrid() {
	if (!this->loaded) {
		reset();
	}

	drawGrid();
}

void Engine::handleNodes(int newX, int newY) {

	if (isNodeValid(40, 40)) {
		this->current_node = this->nodes.at((newY / 40 * this->WIN_W / 40) + newX / 40);
	}

	if (this->last_node == nullptr && this->loaded) {
		this->last_node = this->current_node;
		this->last_node->setColors("impassable");
	}
	else if (this->last_node != nullptr) {

		if (this->last_node != this->current_node) {
			if (isValidMousePosition()) {

				if (!this->pick_start && !this->pick_target) {

					if (strcmp(this->current_node->getType(), "passable") ==1) {
						this->current_node->setColors("impassable");
					}
					else if (strcmp(this->current_node->getType() , "impassable") ==1) {
						this->current_node->setColors("passable");
					}
					else {
						this->current_node->setColors("passable");
					}
				}
				else if (this->pick_start && !this->pick_target) {
					this->current_node->setColors("start");
				}
				else if (!this->pick_start && this->pick_target) {
					this->current_node->setColors("target");
				}

				this->last_node->setColors(this->last_node->getType());
				this->last_node = this->current_node;
			}
		}
	}

}

void Engine::start() {
	std::cout << "Started" << std::endl;

	if (this->complete) {
		clearVisited();
	}

	bool was_paused = this->paused ? true : false;

	this->view_only = true;
	this->playing = true;
	this->paused = false;

	if (this->cost_mode) {
		if (this->use_AStar) {
			if (!was_paused) {
				this->start_node->g = 0;
				this->start_node->f = this->start_node->g + (heuristic(this->start_node, this->target_node) / ((this->start_node->width + this->start_node->height) / 2));

				this->visited_nodes.clear();
				this->open.clear();
				this->open.push_back(this->start_node);
			}

			computePathAStar();

			if (((this->open.empty() && this->target_node != nullptr) || this->target_found) && !this->paused) {
				resolvePath();
			}
		}
		else if (!this->use_AStar) {
			if (!was_paused) {
				this->start_node->g = 0;

				this->visited_nodes.clear();
				this->frontier_dijk.clear();
				this->frontier_dijk.insert(std::make_pair(0, this->start_node));
			}

			computePathDijkstra();

			if (((this->frontier_dijk.empty() && this->target_node != nullptr) || this->target_found) && !this->paused) {
				resolvePath();
			}
		}


	}
	else if (!this->cost_mode) {
		if (this->use_GBFS) {
			if (!was_paused) {
				this->frontier_gbfs.insert(std::make_pair(0, this->start_node));
			}
			
			computePathGBFS();

			if (((this->frontier_gbfs.empty() && this->target_node != nullptr) || this->target_found) && !this->paused) {
				resolvePath();
			}
		}
		else if (!this->use_GBFS) {
			if (!was_paused) {
				this->start_node->setType("visited");
				this->start_node->setColors("start");
				this->frontier.push(this->start_node);
			}

			computePathBFS();

			if (((this->frontier.empty() && this->target_node != nullptr) || this->target_found) && !this->paused) {
				resolvePath();
			}
		}


	}

	if (!this->paused) {
		drawPath();
		this->playing = false;
		this->view_only = false;
		this->complete = true;
	}
	else if (this->paused) {
		this->playing = false;
		this->view_only = true;
		this->complete = false;
	}

}

float Engine::heuristic(Node* a, Node* b) {
	return abs(a->x - b->x) + abs(a->y - b->y);
}

void Engine::computePathDijkstra() {
	while (!this->frontier_dijk.empty()) {

		while (SDL_PollEvent(&this->event)) {
			switch (event.type) {
			case SDL_KEYDOWN:
				switch (event.key.keysym.scancode) {
				case SDL_SCANCODE_P:
					this->paused = true;
					this->playing = false;
					break;
                  default:
                    this->paused = false;
					this->playing = true;
				}
				break;
			}
		}

		if (this->paused) {
			break;
		}

		float cost = this->frontier_dijk.begin()->first;
		Node* currentNode = this->frontier_dijk.begin()->second;
		this->frontier_dijk.erase(this->frontier_dijk.begin());


		if (currentNode == this->target_node) {
			this->target_found = true;
			break;
		}

		for (Node* neighbor : getNeighbors(currentNode)) {

			if (strcmp(neighbor->getType(), "impassable") ==0) {

				double costN = neighbor->cost;
				double distance = cost + costN;

				if (distance < neighbor->g) {
					this->frontier_dijk.erase(std::make_pair(neighbor->g, neighbor));


					neighbor->g = distance;
					neighbor->came_from = currentNode;

					this->frontier_dijk.insert(std::make_pair(neighbor->g, neighbor));
				}

				if (strcmp(neighbor->getType(), "visited") == 0) {

					if (strcmp(neighbor->getType(), "target") ==1) {
						neighbor->setType("visited");
						neighbor->setColors("target");
					}
					else if (strcmp(neighbor->getType(), "start")==1) {
						neighbor->setType("visited");
						neighbor->setColors("start");
					}
					else {
						neighbor->setType("visited");
					}

					this->visited_nodes.push_back(neighbor);
				}


				clearWindow();
				updateGrid();
				updateRenderer();
			}
		}
	}
}

void Engine::computePathAStar() {
	while (!this->open.empty()) {
		while (SDL_PollEvent(&this->event)) {
			switch (event.type) {
			case SDL_KEYDOWN:
				switch (event.key.keysym.scancode) {
				case SDL_SCANCODE_P:
					this->paused = true;
					this->playing = false;
					break;
                  default:
                    this->paused = false;
                    this->playing = true;
				}
				break;
			}

		}

		if (this->paused) {
			break;
		}

		this->open.sort([](const Node* a, const Node* b) { return a->f < b->f;  });

		Node* current_node = this->open.front();

		if (current_node == this->target_node) {
			this->target_found = true;
			break;
		}

		this->open.pop_front();

		for (Node* neighbor : getNeighbors(current_node)) {
			if (strcmp(neighbor->getType(), "impassable") == 0) {
				
				float g = current_node->g + neighbor->cost;

				if (g < neighbor->g) {
					neighbor->came_from = current_node;
					neighbor->g = g;

					int node_width = neighbor->width;
					int node_height = neighbor->height;

					neighbor->h = (heuristic(neighbor, this->target_node) / ((node_width + node_height) / 2));

					neighbor->f = neighbor->g + neighbor->h;

					bool in_open_set = false;

					for (Node* node : this->open) {
						if (node == neighbor) {
							in_open_set = true;
						}
					}

					if (!in_open_set) {
						this->open.push_back(neighbor);
					}
				}

				if (strcmp(neighbor->getType(), "visited") == 0) {

					if (strcmp(neighbor->getType(), "target") == 1) {
						neighbor->setType("visited");
						neighbor->setColors("target");
					}
					else if (strcmp(neighbor->getType(), "start") ==1) {
						neighbor->setType("visited");
						neighbor->setColors("start");
					}
					else {
						neighbor->setType("visited");
					}

					this->visited_nodes.push_back(neighbor);
				}
			}

			clearWindow();
			updateGrid();
			updateRenderer();
		}
	}
}

void Engine::computePathBFS() {
	std::vector<Node*> done_nodes;

	while (!this->frontier.empty()) {

		while (SDL_PollEvent(&this->event)) {
			switch (event.type) {
			case SDL_KEYDOWN:
				switch (event.key.keysym.scancode) {
				case SDL_SCANCODE_P:
					this->paused = true;
					this->playing = false;
					break;
                  default:
                    this->paused = false;
					this->playing = true;
				}
				break;
			}

		}

		if (this->paused) {
			break;
		}

		bool found = false;
		bool shouldContinue = false;

		Node* current_node = this->frontier.front();

		for (Node* node : done_nodes) {
			if (node == this->target_node) {
				found = true;
				break;
			}
			else if (node == current_node) {
				shouldContinue = true;
				break;
			}
		}

		if (shouldContinue) {
			continue;
		}

		if (!found) {
			for (Node* neighbor : getNeighbors(current_node)) {

				if ( (strcmp(neighbor->getType(), "impassable") == 0) && strcmp(neighbor->getType(), "visited")==0) {

					if (strcmp(neighbor->getType(), "target") == 1) {
						neighbor->setType("visited");
						neighbor->setColors("target");
					}
					else {
						neighbor->setType("visited");
					}

					neighbor->came_from = current_node;
					this->frontier.push(neighbor);
					clearWindow();
					updateGrid();
					updateRenderer();
				}
			}

			done_nodes.push_back(current_node);
		}

		this->frontier.pop();
	}
}

void Engine::computePathGBFS() {
	std::vector<Node*> done_nodes;

	bool found = false;
	bool shouldContinue = false;

	while (!this->frontier_gbfs.empty()) {

		while (SDL_PollEvent(&this->event)) {
			switch (event.type) {
			case SDL_KEYDOWN:
				switch (event.key.keysym.scancode) {
				case SDL_SCANCODE_P:
					this->paused = true;
					this->playing = false;
					break;
                  default:
                    this->paused = false;
					this->playing = true;
				}
				break;
			}

		}

		if (this->paused) {
			break;
		}

		Node* current_node = this->frontier_gbfs.begin()->second;
		this->frontier_gbfs.erase(this->frontier_gbfs.begin());

		for (Node* node : done_nodes) {
			if (node == this->target_node) {
				found = true;
				break;
			}
			else if (node == current_node) {
				shouldContinue = true;
				break;
			}

		}

		if (shouldContinue) {
			shouldContinue = false;
			continue;
		}

		if (!found) {
			for (Node* neighbor : getNeighbors(current_node)) {

				if ((strcmp(neighbor->getType(), "impassable") == 0) && (strcmp(neighbor->getType(), "visited")) ==0) {

					if (strcmp(neighbor->getType(), "target")==1) {
						neighbor->setType("visited");
						neighbor->setColors("target");
					}
					else if (strcmp(neighbor->getType(), "start") ==1) {
						neighbor->setType("visited");
						neighbor->setColors("start");
					}
					else {
						neighbor->setType("visited");
					}

					neighbor->came_from = current_node;

					float h = heuristic(this->target_node, neighbor) / 40;

					neighbor->h = h;

					this->frontier_gbfs.insert(std::make_pair(neighbor->h, neighbor));
					clearWindow();
					updateGrid();
					updateRenderer();
				}
			}

			done_nodes.push_back(current_node);
		}

	}
}

void Engine::resolvePath() {
	this->path = std::vector<Node*>();

	Node* current_node = this->target_node;

	while (current_node != this->start_node) {
		this->path.push_back(current_node);
		current_node = current_node->came_from;
	}

	this->path.push_back(this->start_node);
	this->complete = true;
}

void Engine::drawPath() {
	for (Node* node : this->nodes) {
		if (strcmp(node->getType(), "visited") ==1) {
			bool isPath = false;

			for (Node* pathNode : this->path) {
				if (node == pathNode) {
					isPath = true;
					break;
				}
			}

			if (!isPath) {
				if (this->cost_mode) {
					float cost = node->cost;
					node->setType("passable");
					node->cost = cost;
				}
				else if (!this->cost_mode) {
					node->setType("passable");
				}
			}
			else if (isPath && node == this->start_node) {
				this->start_node->setType("start");
			}
			else if (isPath && node == this->target_node) {
				this->target_node->setType("target");
			}
		}
	}

	this->target_found = false;
}

void Engine::updateCost() {
	if (this->cost_mode) {
		if (strcmp(this->current_node->getType(), "passable") == 1){

			if (this->current_node->cost < 4) {
				this->current_node->cost += 1;
			}
			else if (this->current_node->cost == 4) {
				this->current_node->cost += 1;
				this->current_node->setType("impassable");
			}

		}
	}
}

void Engine::toggleCostMode() {
	this->cost_mode = !this->cost_mode;

	if (this->cost_mode) {
		this->cost_font = TTF_OpenFont("fonts/Lato-Regular.ttf", 25);
	}
	else if (!this->cost_mode) {
		TTF_CloseFont(this->cost_font);
	}

	reset();
}

void Engine::toggleViewOnly() {
	this->last_node->setType(this->last_node->getType());
	this->view_only = !this->view_only;
}

void Engine::toggleTargetMode() {
	if (!this->view_only) {
		this->pick_target = !this->pick_target;

		if (this->pick_target) {
			this->current_node->setColors("target");
		}
		else if (!this->pick_target) {
			this->current_node->setColors((strcmp(this->current_node->getType(), "passable") ==1) ? "impassable" : "passable");
		}
	}
}

void Engine::toggleStartMode() {
	if (!this->view_only) {
		this->pick_start = !this->pick_start;
	}

	if (this->pick_start) {
		this->current_node->setColors("start");
	}
	else if (!this->pick_start) {
		this->current_node->setColors((strcmp(this->current_node->getType(), "passable") == 1) ? "impassable" : "passable");
	}
}

void Engine::reset() {
	this->view_only = false;
	this->playing = false;
	this->paused = false;
	this->target_found = false;
	this->last_node_changed = nullptr;
	this->start_node = nullptr;
	this->target_node = nullptr;
	this->last_node = nullptr;

	resetGrid();
	drawGrid();

	this->loaded = true;

	std::cout << "Reset" << std::endl;
}

void Engine::resetGrid() {
	this->nodes = std::vector<Node*>();

	int idx = 0;

	for (int y = 0; y < this->WIN_H; y += 40) {
		for (int x = 0; x < this->WIN_W; x += 40) {
			Node* node = new Node("passable", x, y);
			node->index = idx;

			this->nodes.push_back(node);
			this->nodes.back()->index = idx;
			idx++;
		}
	}
}

void Engine::drawGrid() {
	for (Node* node : this->nodes) {
		drawNode(node);
	}
}

void Engine::handleMouse(bool button_down, int button_pressed) {
	if (button_down) {
		if (button_pressed == SDL_BUTTON_LEFT) {
			this->is_held = true; // Reference variable for checking if the left mouse button is being held.
		}
		else if (button_pressed == SDL_BUTTON_RIGHT) { // If right mouse button is clicked when hovering a Node inside "cost mode", increase the Node's cost by 1. If it reaches 5, becomes "impassable".
			updateCost();
		}
	}
	else if (!button_down) {
		this->is_held = false;
	}

}

void Engine::updateMousePosition() {
	if (!this->view_only) {
		SDL_GetGlobalMouseState(&this->mouseX, &this->mouseY); // Updates mouseX and mouseY variables with the mouse position within the display.
		SDL_GetWindowPosition(this->window, &this->xPos, &this->yPos); // Updates xPos and yPos variables with the window position within the display.

		this->newX = this->mouseX - this->xPos - 1; // Updates newX with the mouse X position within the window. (With 0 being the leftmost points of the window).
		this->newY = this->mouseY - this->yPos - 1; // Updates newY with the mouse Y position within the window. (With 0 being the topmost points of the window).

		handleNodes(this->newX, this->newY); // Handles node colors and states when they're hovered over.
	}
}

void Engine::handleMouseClick() {
	if (!this->view_only) {

		if (this->is_held) {

			Node* current_node = this->current_node;

			// Checks if the Node currently being hovered over has changed since the last time it was updated (implementation of "painting" Nodes) and if the Node's index does not exceed the Nodes array size.
			if (current_node != this->last_node_changed && current_node->index < this->nodes.size()) {

				// If the active selection mode is not both start and target points.
				if (!this->pick_start && !this->pick_target) {


					// If Node is not passable, make it passable and update its cost to 1.
					if (strcmp(current_node->getType(), "passable")==0) {
						current_node->setType("passable");

						if (this->cost_mode) {
							current_node->cost = 1;
						}
					}

					// If Node is passable, make it impassable and update its cost to 5.
					else if (strcmp(current_node->getType(), "passable")==1) {
						current_node->setType("impassable");

						if (this->cost_mode) {
							current_node->cost = 5;
						}
					}

					// Updates the last changed node to be the current one.
					this->last_node_changed = current_node;
				}

				// If the active selection mode is start but not target point.
				else if (this->pick_start && !this->pick_target) {

					// Updates the Node to become the starting Node.
					current_node->setType("start");
					current_node->cost = 1;
					this->pick_start = false;

					// If starting Node has already been set before, set the previous starting Node to passable and update the starting Node index to be the current Node's
					if (this->start_node != nullptr) {
						this->start_node->setType("passable");
						this->start_node = current_node;
					}

					// If starting Node has not been set before, update the starting Node index to be the current Node's
					else {
						this->start_node = current_node;
					}

					// Updates the last changed node to be the current one.
					this->last_node_changed = current_node;
				}

				// If the active selection mode is target but not target start.
				else if (!this->pick_start && this->pick_target) {

					// Updates the Node to become the target Node.
					current_node->setType("target");
					current_node->cost = 1;
					this->pick_target = false;

					// If target Node has already been set before, set the previous target Node to passable and update the target Node index to be the current Node's
					if (this->target_node != nullptr) {
						this->target_node->setType("passable");
						this->target_node = current_node;
					}

					// If target Node has not been set before, update the target Node index to be the current Node's
					else {
						this->target_node = current_node;
					}

					// Updates the last changed node to be the current one.
					this->last_node_changed = current_node;
				}
			}
		}
	}
}

void Engine::changeAlgorithm() {
	if (this->cost_mode) {
		this->use_AStar = !this->use_AStar;
		std::cout << "Current algorithm: " << (this->use_AStar ? "A*" : "Dijkstra's") << std::endl;
	}
	else if (!this->cost_mode) {
		this->use_GBFS = !this->use_GBFS;
		std::cout << "Current algorithm: " << (this->use_GBFS ? "Greedy Best-First Search" : "Breadth-First Search") << std::endl;
	}
}

void Engine::clearVisited() {
	if (this->complete && this->cost_mode) {

		for (Node* visited_node : this->visited_nodes) {
			float cost = visited_node->cost;
			visited_node->setType("passable");
			visited_node->cost = cost;
		}

		this->start_node->setType("start");
		this->target_node->setType("target");

		this->complete = false;
		this->last_node_changed = nullptr;
		this->last_node = nullptr;
	}
	else if (this->complete && !this->cost_mode) {
		for (Node* path_node : this->path) {
			if (strcmp(path_node->getType(), "visited")==1) {
				if (path_node == this->start_node) {
					path_node->setType("start");
				}
				else if (path_node == this->target_node) {
					path_node->setType("target");
				}
				else {
					path_node->setType("passable");
				}
			}
		}

		this->complete = false;
		this->last_node_changed = nullptr;
		this->last_node = nullptr;
	}

	this->target_found = false;
}

bool Engine::isNodeValid(int width, int height) {
	return (newY / height * this->WIN_W / width) + newX / width < static_cast<int>(this->nodes.size());
}

bool Engine::isValidMousePosition() {
	return (newX >= 0 && newY >= 0 && newX < this->WIN_W && newY < this->WIN_H);
}

bool Engine::isValidLeftNeighbor(int index) {
	return (index % (this->nodes_per_row_IDX + 1) != 0);
}

bool Engine::isValidTopNeighbor(int index) {
	return (index > this->nodes_per_row_IDX);
}

bool Engine::isValidRightNeighbor(int index) {
	return (index % (this->nodes_per_row_IDX + 1) != 19);
}

bool Engine::isValidBottomNeighbor(int index) {
	return (index <= this->node_count_IDX - this->nodes_per_row_IDX - 1);
}

std::vector<Node*> Engine::getNeighbors(Node* node) {
	std::vector<Node*> neighbors;

	if (isValidLeftNeighbor(node->index)) {
		Node* left_neighbor = this->nodes.at(node->index - 1);
		neighbors.push_back(left_neighbor);
	}

	if (isValidTopNeighbor(node->index)) {
		Node* top_neighbor = this->nodes.at(node->index - (this->nodes_per_row_IDX + 1));
		neighbors.push_back(top_neighbor);
	}

	if (isValidRightNeighbor(node->index)) {
		Node* right_neighbor = this->nodes.at(node->index + 1);
		neighbors.push_back(right_neighbor);
	}

	if (isValidBottomNeighbor(node->index)) {
		Node* bottom_neighbor = this->nodes.at(node->index + this->nodes_per_row_IDX + 1);
		neighbors.push_back(bottom_neighbor);
	}

	return neighbors;
}

int Engine::getWidth() {
	return this->WIN_W;
}

int Engine::getHeight() {
	return this->WIN_H;
}

SDL_Event Engine::getEvent() {
	return this->event;
}