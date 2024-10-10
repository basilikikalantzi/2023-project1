#include "raylib.h"

#include "state.h"
#include "interface.h"

// Assets
Texture star_img;
Texture ball_img;
Sound game_over_snd;


void interface_init() {
	// Αρχικοποίηση του παραθύρου
	InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "game");
	SetTargetFPS(60);
    InitAudioDevice();

	// Φόρτωση εικόνων και ήχων
	ball_img = LoadTextureFromImage(LoadImage("assets/ball.png"));
	star_img = LoadTextureFromImage(LoadImage("assets/star.png"));
	game_over_snd = LoadSound("assets/game_over.mp3");
}

void interface_close() {
	CloseAudioDevice();
	CloseWindow();
}

// Draw game (one frame)
void interface_draw_frame(State state) {
	BeginDrawing();

	// Καθαρισμός, θα τα σχεδιάσουμε όλα από την αρχή
	ClearBackground(RAYWHITE);

	// Σχεδιάζουμε την μπάλα και τις πλατφόρμες
	StateInfo info = state_info(state);
	float ball_x = info->ball->rect.x;
	float x_offset = 100 - ball_x;		// H απόσταση μεταξύ των θέσεων των αντικειμένων στον χάρτη και στην οθόνη
	
	// ball_x + x_offset είναι η θέση χ της μπάλας στον χάρτη συν την απα΄ραίτητη απόσταση για να παρασταθεί σωστά στην οθόνη
	// info->ball->rect.y είναι η θέση y της μπάλας στον χάρτη και δεν χρειάζεται η απόσταση x_offset αφού η οθόνη μετακινείται προς τα δεξιά
	DrawTexture(ball_img, ball_x + x_offset, info->ball->rect.y, RED); 

	// Η λίστα με όλα τα αντικείμενα σε απόσταση δύο οθονών απο τη μπάλα 
	List objs = state_objects(state, ball_x - 2*SCREEN_WIDTH, ball_x + 2*SCREEN_WIDTH);
	for(ListNode node = list_first(objs) ; node!=LIST_EOF ; node=list_next(objs, node)){
		Object obj = list_node_value(objs, node);
		// Σχεδίαζω την πλατφόρμα
		// obj->rect.x + x_offset είναι η θέση χ της πλατφόρμας στον χάρτη συν την απα΄ραίτητη απόσταση για να παρασταθεί σωστά στην οθόνη
		// obj->rect.y είναι η θέση y της πλατφόρμας στον χάρτη και δεν χρειάζεται η απόσταση x_offset αφού η οθόνη μετακινείται προς τα δεξιά
		if(obj->type == PLATFORM){
			if(obj->unstable){	// Αν είναι ασταθής γίνεται κόκκινη
				DrawRectangle(obj->rect.x + x_offset, obj->rect.y, obj->rect.width, obj->rect.height, RED);
			}
			else{				// Αλλιώς γινεται μπλε
				DrawRectangle(obj->rect.x + x_offset, obj->rect.y, obj->rect.width, obj->rect.height, BLUE);
			}
		}
		// Σχεδίαζω το αστέρι με την ίδια λογική
		else{
			DrawTexture(star_img, obj->rect.x + x_offset, obj->rect.y, YELLOW);
		}
	}


	// Σχεδιάζουμε το σκορ και το FPS counter
	DrawText(TextFormat("%04i", info->score), 20, 20, 40, GRAY);
	DrawFPS(SCREEN_WIDTH - 80, 0);

	// Αν το παιχνίδι έχει τελειώσει, σχεδιάζουμαι το μήνυμα για να ξαναρχίσει
	if (!info->playing) {
		DrawText(
			"PRESS [ENTER] TO PLAY AGAIN",
			 GetScreenWidth() / 2 - MeasureText("PRESS [ENTER] TO PLAY AGAIN", 20) / 2,
			 GetScreenHeight() / 2 - 50, 20, GRAY
		);
	}

	// Αν το παιχνίδι είναι σε pause, σχεδιάζουμε το μήνυμα για να βγει απο το pause
	if(info->paused){
		DrawText(
			"YOU ARE IN PAUSE PRESS [P] TO PLAY AGAIN",
			 GetScreenWidth() / 2 - MeasureText("YOU ARE IN PAUSE PRESS [P] TO PLAY AGAIN", 20) / 2,
			 GetScreenHeight() / 2 - 50, 20, GRAY
		);
	}

	// Ηχος, αν είμαστε στο frame που συνέβη το game_over
	if(!info->playing)
		PlaySound(game_over_snd);

	EndDrawing();
}