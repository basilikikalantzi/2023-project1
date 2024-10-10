#include <stdio.h>
#include "raylib.h"

#include "interface.h"
#include "state.h"

State state;

void update_and_draw() {
	// Για να λειτουργούν τα πλήκτρα
	struct key_state keys;
	keys.left = IsKeyDown(KEY_LEFT);
	keys.right = IsKeyDown(KEY_RIGHT);
	keys.up = IsKeyDown(KEY_UP);
	keys.down = IsKeyDown(KEY_DOWN);
	keys.enter = IsKeyDown(KEY_ENTER);
	keys.p = IsKeyPressed(KEY_P);
	keys.n = IsKeyPressed(KEY_N);
	state_update(state, &keys);
	interface_draw_frame(state);
}

int main() {
	state = state_create();		// Δημιουργεί την κατάσταση
	interface_init();			// Και την ζωγραφίζει

	// Η κλήση αυτή καλεί συνεχόμενα την update_and_draw μέχρι ο χρήστης να κλείσει το παράθυρο
	start_main_loop(update_and_draw);

	interface_close();	// 	Κλείνει το Interface

	return 0;
}
