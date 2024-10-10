#include <stdlib.h>
#include <stdio.h>
#include "ADTSet.h"
#include "ADTList.h"
#include "state.h"
#include "set_utils.h"
#include "common_types.h"



// Συγκρίνει 2 αντικείμενα με βάση τη θέση x τους
int compare_object(Pointer obj1, Pointer obj2) {
	float x1;
	if(obj1 != NULL){
		Object iobj1 = obj1;
		x1 = iobj1->rect.x;
	}
	else{
		x1=0;
	}
	Object iobj2 = obj2;
	float x2 = iobj2->rect.x;
	
    return x1 - x2;       // αρνητικός, μηδέν ή θετικός, ανάλογα με τη διάταξη των x1,x2
}

struct state {
	Set objects;			// περιέχει στοιχεία Object (Πλατφόρμες, Αστέρια)
	struct state_info info;	// Γενικές πληροφορίες για την κατάσταση του παιχνιδιού
	float speed_factor;		// Πολλαπλασιαστής ταχύτητς (1 = κανονική ταχύτητα, 2 = διπλάσια, κλπ)
};

// Δημιουργεί και επιστρέφει ένα αντικείμενο

static Object create_object(ObjectType type, float x, float y, float width, float height, VerticalMovement vert_mov, float speed, bool unstable) {
	Object obj = malloc(sizeof(*obj));
	obj->type = type;
	obj->rect.x = x;
	obj->rect.y = y;
	obj->rect.width = width;
	obj->rect.height = height;
	obj->vert_mov = vert_mov;
	obj->vert_speed = speed;
	obj->unstable = unstable;
	return obj;
}

static void add_objects(State state, float start_x) {
	// Προσθέτουμε PLATFORM_NUM πλατφόρμες, με τυχαία χαρακτηριστικά.

	for (int i = 0; i < PLATFORM_NUM; i++) {
		Object platform = create_object(
			PLATFORM,
			start_x + 150 + rand() % 80,						// x με τυχαία απόσταση από το προηγούμενο στο διάστημα [150, 230]
			SCREEN_HEIGHT/4 + rand() % SCREEN_HEIGHT/2,			// y τυχαία στο διάστημα [SCREEN_HEIGHT/4, 3*SCREEN_HEIGHT/4]
			i == 0 ? 250 : 50 + rand()%200,						// πλάτος τυχαία στο διάστημα [50, 250] (η πρώτη πάντα 250)
			20,													// ύψος
			i < 3 || rand() % 2 == 0 ? MOVING_UP : MOVING_DOWN,	// τυχαία αρχική κίνηση (οι πρώτες 3 πάντα πάνω)
			0.6 + 3*(rand()%100)/100,							// ταχύτητα τυχαία στο διάστημα [0.6, 3.6]
			i > 0 && (rand() % 10) == 0							// το 10% (τυχαία) των πλατφορμών είναι ασταθείς (εκτός από την πρώτη)
		);
        set_insert(state->objects, platform);

		// Στο 50% των πλατφορμών (τυχαία), εκτός της πρώτης, προσθέτουμε αστέρι
		if(i != 0 && rand() % 2 == 0) {
			Object star = create_object(
				STAR,
				start_x + 200 + rand() % 60,				// x με τυχαία απόσταση από την προηγούμενη πλατφόρμα στο διάστημα [200,260]
				SCREEN_HEIGHT/8 + rand() % SCREEN_HEIGHT/2,	// y τυχαία στο διάστημα [SCREEN_HEIGHT/8, 5*SCREEN_HEIGHT/8]
				30, 30,										// πλάτος, ύψος
				IDLE,										// χωρίς κίνηση
				0,										 	// ταχύτητα 0
				false										// 'unstable' πάντα false για τα αστέρια
			);
			set_insert(state->objects, star);
		}

		start_x = platform->rect.x + platform->rect.width;	// μετακίνηση των επόμενων αντικειμένων προς τα δεξιά
	}
}

// Δημιουργεί και επιστρέφει την αρχική κατάσταση του παιχνιδιού
State state_create() {
	// Δημιουργία του state
	State state = malloc(sizeof(*state));

	// Γενικές πληροφορίες
	state->info.playing = true;				// Το παιχνίδι ξεκινάει αμέσως
	state->info.paused = false;				// Χωρίς να είναι paused.
	state->speed_factor = 1;				// Κανονική ταχύτητα
	state->info.score = 0;				// Αρχικό σκορ 0

	// Δημιουργούμε το set των αντικειμένων, και προσθέτουμε αντικείμενα
	// ξεκινώντας από start_x = 0.
	state->objects = set_create(compare_object, NULL);
	add_objects(state, 0);

	// Δημιουργούμε την μπάλα τοποθετώντας τη πάνω στην πρώτη πλατφόρμα
	SetNode first_platform_node = set_first(state->objects);
	Object first_platform = set_node_value(state->objects, first_platform_node);
	state->info.ball = create_object(
		BALL,
		first_platform->rect.x,			// x στην αρχή της πλατφόρμας
		first_platform->rect.y - 40,	// y πάνω στην πλατφόρμα
		45, 45,							// πλάτος, ύψος
		IDLE,							// χωρίς αρχική κατακόρυφη κίνηση
		0,								// αρχική ταχύτητα 0
		false							// "unstable" πάντα false για την μπάλα
	);

	return state;
}


// Επιστρέφει τις βασικές πληροφορίες του παιχνιδιού στην κατάσταση state
StateInfo state_info(State state) {
	return &state->info;
}

// Επιστρέφει μια λίστα με όλα τα αντικείμενα του παιχνιδιού στην κατάσταση state,
// των οποίων η συντεταγμένη x είναι ανάμεσα στο x_from και x_to.
List state_objects(State state, float x_from, float x_to) { //υλοποιημένη
	List obj_list = list_create(NULL);
	Object obj_from = create_object(STAR,x_from,0,30,30,IDLE,0,false);
	Object obj_to = create_object(STAR,x_to,0,30,30,IDLE,0,false);
	obj_from = set_find_eq_or_greater(state->objects,obj_from);
	if(obj_from == NULL) return obj_list;
	obj_to = set_find_eq_or_smaller(state->objects,obj_to);
	if(obj_to == NULL) return obj_list;
	// if(obj_from == NULL || obj_to == NULL){
	// 	return obj_list;
	// }
	for(SetNode node = set_find_node(state->objects,obj_from);
		node != set_find_node(state->objects,obj_to);
		node = set_next(state->objects, node) ){
	list_insert_next(obj_list,LIST_BOF,set_node_value(state->objects,node));
	}
	return obj_list;
}


// Ενημερώνει την κατάσταση state του παιχνιδιού μετά την πάροδο 1 frame.
// Το keys περιέχει τα πλήκτρα τα οποία ήταν πατημένα κατά το frame αυτό.
void state_update(State state, KeyState keys) {
	
	//PAUSE-1 NEW FRAME
	if(!state->info.paused && keys->p){			// Aν πατηθεί το P και το παιχνίδι δεν είναι σε pause
		state->info.paused = true;				// Μπαίνει σε κατάσταση pause
	}
	else if(state->info.paused && !keys->p){	// Aν δεν πατηθεί το P και το παιχνίδι είναι σε pause παραμένει έτσι 
		if(keys->n){							// Aν πατηθεί το N
			state->info.paused = false;			// Βγαίνει απο την κατάσταση pause
			state_update(state, keys);			// Ενημερώνεται για ένα frame 
			state->info.paused = true;			// Και ξαναμπαίνει σε κατάσταση pause
		}
		return;
	}
	else if(state->info.paused && keys->p){		// Aν πατηθεί το P και το παιχνίδι είναι σε pause
		state->info.paused = false;				// Βγαίνει απο την κατάσταση pause
	}

	
	// Oριζόντια κίνηση μπάλας
	if(keys->right) {						//Αν είναι πατημένο το δεξί βέλος τότε η μπάλα μετακινείται 6 pixels
		state->info.ball->rect.x += 6;
	}
	else if(keys->left){					// Αν είναι πατημένο το αριστέρο βέλος τότε η μπάλα μετακινείται 1 pixel
		state->info.ball->rect.x += 1;
	}
	else{									// Αν δεν είναι πατημένο κανένα βέλος τότε η μπάλα μετακινείται 4 pixels
		state->info.ball->rect.x += 4;
	}

	
	// Κατακόρυφη κίνηση μπάλας
	if(state->info.ball->vert_mov == IDLE){		// Αν βρίσκεται σε ηρεμία
		if(keys->up){							// Αν είναι πατημένο το πάνω βέλος 
			state->info.ball->vert_speed = 17;  // Αρχικοποιεί την αρχική κατακόρυφη ταχύτητα της με 17
			state->info.ball->vert_mov = JUMPING; // Μπαίνει σε κατάσταση άλματος 
		}
	}
	else if(state->info.ball->vert_mov == JUMPING){		// Αν βρίσκεται σε άλμα
		if(state->info.ball->vert_speed > 0.5 && state->info.ball->rect.y>0){ // Άν η κατακόρυφη ταχύτητα δεν φτάσει το 0.5
																	// Και η μπάλα δεν ακουμπήσει το άνω μέρος της οθόνης
			state->info.ball->rect.y -= state->info.ball->vert_speed;// Μετακινείται προς τα πάνω τόσα pixels όσα η κατακόρυφη ταχύτητά της	  
			state->info.ball->vert_speed *= 0.85;				// Η ταχύτητα μειώνετα στο 85% της προηγούμενης τιμής
		}
		else{													// Όταν η κατακόρυφη ταχύτητα φτάσει το 0.5
			if(state->info.ball->rect.y<=0){
				state->info.ball->vert_speed = 0.5;	  // Για να μην πέφτει γρήγορα όταν ακουμπάει το άνω μέρος της οθόνης
			}
			state->info.ball->vert_mov = FALLING;	  // Μπαίνει σε κα΄τάσταση πτώσης
		}
	}
	else if(state->info.ball->vert_mov == FALLING){  // Αν βρίσκεται σε πτώση
		state->info.ball->rect.y += state->info.ball->vert_speed; // Μετακινείται προς τα κάτω τόσα pixels όσα η κατακόρυφη ταχύτητά της
		if(state->info.ball->vert_speed >7){ 		// Άν η κατακόρυφη ταχύτητα γίνει μεγαλύτερη από 7
			state->info.ball->vert_speed = 7;   	// Παραμένει στο 7, αφού 7 η max ταχύτητα
		}
		state->info.ball->vert_speed += 0.1;		// Η ταχήτητα αυξάνεται κατά 10%
	}
	
	List objs = state_objects(state, state->info.ball->rect.x - 2*SCREEN_WIDTH, state->info.ball->rect.x + 2*SCREEN_WIDTH);
	
	// Κατακόρυφη κ΄΄ινηση πλατφορμών
	for(ListNode node = list_first(objs) ; node != LIST_EOF ; node=list_next(objs, node)){
		Object obj = list_node_value(objs, node);
		
		if(obj->type == PLATFORM){						
			if(obj->unstable && CheckCollisionRecs(obj->rect, state->info.ball->rect)){ // Άν η πλατφόρμα είναι ασταθής και γίνει σύγκρουση με τη μπάλα
				obj->vert_mov = FALLING;				// Η πλατφόρμα μπαίνει σε κατάσταση FALLING
			}

			if(obj->vert_mov == MOVING_UP){				// Αν κινείται προς τα πάνω
				if(obj->rect.y >= SCREEN_HEIGHT/4){
					obj->rect.y -= obj->vert_speed; // Μετακινείται προς τα πάνω τόσα pixels όσα η κατακόρυφη ταχύτητά της.
				}
				else{								// Αν περάσει το SCREEN_HEIGHT/4 
					obj->vert_mov = MOVING_DOWN;	// Αλλάζει σε MOVING_DOWN
				}
			}
			else if(obj->vert_mov == MOVING_DOWN){	// Αν κινείται προς τα κάτω 
 
					if(obj->rect.y <= 3*SCREEN_HEIGHT/4){ 
						obj->rect.y += obj->vert_speed;	// Μετακινείται προς τα κάτω τόσα pixels όσα η κατακόρυφη ταχύτητά της
					}
					else{								// Αν περάσει τo 3*SCREEN_HEIGHT/4
						obj->vert_mov = MOVING_UP;		// Αλλάζει σε MOVING_UP
					}
			}
			else if(obj->vert_mov == FALLING){			// Αν βρίσκεται σε πτώση
				obj->rect.y += 4;						// Μετακινείται προς τα κάτω 4 pixels μέχρι να συγκρουστεί με το κάτω μέρος της οθόνης
			}
		}
	}


	// Συμπεριφορά μπάλας σε κατακόρυφη ηρεμία
	if(state->info.ball->vert_mov == IDLE){		// Αν η μπάλα βρίσκεται σε ηρεμία 				
		bool flag = false;
		for(ListNode node = list_first(objs) ; node!= LIST_EOF ; node = list_next(objs, node)){
			Object obj = list_node_value(objs, node);
			if(obj->type == PLATFORM){							// Αν το αντικείμενο είναι πλατφόρμα
					// Και αν η συντεταγμένη της x βρίσκεται στα όρια κάποιας πλατφόρμας
					if(state->info.ball->rect.x >= obj->rect.x && state->info.ball->rect.x <= obj->rect.x + obj->rect.width){
					state->info.ball->rect.y = obj->rect.y - 40;	// Η μπάλα ακολουθεί σε ύψος την πλατφόρμα
																	//(- obj->rect.height ώστε να βρίσκεται πάνω στην πλατφόρμα)
					flag = true;	// Αν βρεθεί έστω και μία πλατφόρμα δεν χρειάζεται να ελεγχθούν οι άλλες
					break;			// Οπότε βγαίνει από το loop
				}

			}	
		}
		if (!flag){					// Αν δεν βρεθεί καμία πλατφόρμα
			state->info.ball->vert_mov = FALLING;	// Η μπάλα αρχίζει να πέφτει
			state->info.ball->vert_speed = 1.5;
		}
	}


	// Συγκρούσεις
	if(state->info.ball->rect.y >= SCREEN_HEIGHT){	// Αν η μπάλα ακουμπήσει το κάτω μέρος της οθόνης 
		state->info.playing = false;				// Σταματάει το παιχνίδι
	}

	for(ListNode node = list_first(objs) ; node!=LIST_EOF ; node = list_next(objs, node)){
		Object obj = list_node_value(objs, node);
		
		// Σύγκρουση μπάλας με αστέρι
		if(obj->type == STAR && CheckCollisionRecs(state->info.ball->rect, obj->rect)){	// Αν το αντικείμενο είναι αστέρι και γίνει η σύγκρουση	
			set_remove(state->objects, obj); // Αφαιρείται σπό το set
			state->info.score += 10;		 // Και αυξάνεται το σκορ
		}
		
		// Σύγκρουση πλατφόρμας με το κάτω μέρος της οθόνης
		// Αν το αντικείμενο είναι  ασταθής πλατφορμα και φτάσει το κάτω μέρος της οθόνης
		if(obj->type == PLATFORM && obj->vert_mov == FALLING && obj->rect.y >= SCREEN_HEIGHT){
			set_remove(state->objects, obj);	// Αφαιρείται σπό το set
		}

		// Σύγκρουση μπάλας με πλατφόρμα
		// Αν μπάλα είναι σε πτώση και βρίσκεται πα΄νω απο την πλατφόρμα και συγκρουστεί με πλατφόρμα
		// Κανονικά θα έπρεπε να βάλω +40(obj->rect.height) αλλά έβαλα +30 για να υπάρχει ένα περιθώριο και να μην πέφτει πολ΄υ εύκολα 	
		if(state->info.ball->vert_mov == FALLING && obj->type == PLATFORM 
			&& CheckCollisionRecs(state->info.ball->rect, obj->rect) && state->info.ball->rect.y + 30 < obj->rect.y){
				state->info.ball->vert_mov = IDLE;	// Η μπάλα σταματάει να πέφτει και μπαίνει σε κατάσταση IDLE
		}	
		
	}
	Object last_platform_level = set_node_value(state->objects, set_last(state->objects)); // Βρίσκει την τελευταία πλατφόρμα της πίστας
	if(state->info.ball->rect.x > last_platform_level->rect.x - SCREEN_WIDTH){ // Αν η μπάλα φτάσει σε απόσταση μίας οθόνης από την τελευταία πλατφόρμα
		add_objects(state, last_platform_level->rect.x );	// Κάνει επέκταση από τη θέση της τελευταίας πλατφόρμας 
		state->speed_factor *= 1.1;					// Η ταχύτητα του παιχνιδιού γίνεται 10% μεγαλύτερη
		state->info.ball->vert_speed *= 1.1;		// Η ταχύτητα της μπάλας γίνεται 10% μεγαλύτερη
		// Δημιουργεί μία λίστα με τα αντικείμενα που μόλις προστέθηκαν
		objs = state_objects(state, last_platform_level->rect.x, last_platform_level->rect.x + PLATFORM_NUM * SCREEN_WIDTH);
		for(ListNode node=list_first(objs) ; node!=LIST_EOF ; node=list_next(objs, node)){ // Διασχίζει αυτά τα αντικείμενα
			Object obj = list_node_value(objs, node);
			obj->vert_speed *=1.1;		// Η ταχύτητα των πλατφορμών γίνεται 10% μεγαλύτερη
		}
	}

	if(!state->info.playing && keys->enter){		// Αν το παιχνίδι έχει τελειώσει και πατηθεί enter
		state_destroy(state);						// Καταστρέφει την προηγούμενη κατάσταση
		state_create(state);						// Ξαναρχίζει από την αρχή 
	}
}

	

// Καταστρέφει την κατάσταση state ελευθερώνοντας τη δεσμευμένη μνήμη.
void state_destroy(State state) {
	free(state);
	// Προς υλοποίηση
}