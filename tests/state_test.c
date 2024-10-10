//////////////////////////////////////////////////////////////////
//
// Test για το state.h module
//
//////////////////////////////////////////////////////////////////

#include "acutest.h"			// Απλή βιβλιοθήκη για unit testing

#include "state.h"
#include "set_utils.h"
#include <stdio.h>

int* create_ints(int value) {
    int* pointer = malloc(sizeof(int));     // δέσμευση μνήμης
    *pointer = value;                       // αντιγραφή του value στον νέο ακέραιο
    return pointer;
}

// Συγκρίνει 2 ακεραίους
 int compare_ints(Pointer a, Pointer b){
    int* x = a;
    int* y = b;
    return *x - *y;		// αρνητικός, μηδέν ή θετικός, ανάλογα με τη διάταξη των a,b
}



void test_state_create() {

	State state = state_create(); 	// Ελέγχει αν η state_create δημιουργεί και επιστρέφει την αρχική κατάσταση 
	TEST_ASSERT(state != NULL);
	

	StateInfo info = state_info(state); // Ελέγχει αν  η state_info επιστρέφει τις βασικές πληροφορίες του παιχνιδιού 
	TEST_ASSERT(info != NULL);			// στην κατάσταση state

	// Ελέγχονται αν πληρούνται οι γενικές πληροφορίες της αρχικής κατάστασης του παιχνιδιού
	TEST_ASSERT(info->playing);			
	TEST_ASSERT(!info->paused);			
	TEST_ASSERT(info->score == 0);		
	TEST_ASSERT(info->ball->type == BALL);
	TEST_ASSERT(info->ball->rect.x >= 150);
	TEST_ASSERT(info->ball->rect.x <= 230);
	TEST_ASSERT(info->ball->rect.y >= SCREEN_HEIGHT/4);
	TEST_ASSERT(info->ball->rect.y <= 3*SCREEN_HEIGHT/4);
	TEST_ASSERT(info->ball->rect.height == 45);
	TEST_ASSERT(info->ball->rect.width == 45);
	TEST_ASSERT(info->ball->vert_mov == IDLE);
	TEST_ASSERT(info->ball->vert_speed == 0);
	TEST_ASSERT(!info->ball->unstable);


	List objs = state_objects(state, 0, 900); // Ελ΄έγχει αν η state_objects επιστρέφει τα αντικείμενα του παιχνιδιού 
	TEST_ASSERT(objs != NULL);				  // των οποίων η συντεταγμένη x είναι ανάμεσα στο 0 και 900
	TEST_ASSERT(list_size(objs) <= 7); // Η κάθε πλατφόρμα έχει απόσταση 150, άρα σε μία οθόνη 900 χωράνε το πολ΄υ 5 πλατφόρμες και 50% * 5 = 2,5 (στρογγυλ΄ά 2) αστέρια
	objs = state_objects(state, 2700, 3600);	// Κάνει το ίδιο για τα αντικειμενα στην οθόνη 2700-3600
	TEST_ASSERT(objs != NULL);
	TEST_ASSERT(list_size(objs) <= 7);	
	objs = state_objects(state, 7200, 8100);	// Κάνει το ίδιο για τα αντικειμενα στην οθόνη 7200-8100
	TEST_ASSERT(objs != NULL);
	TEST_ASSERT(list_size(objs) <= 7);
}


// Ενημερώνει την κατάσταση state του παιχνιδιού μετά την πάροδο 1 frame.
// Το keys περιέχει τα πλήκτρα τα οποία ήταν πατημένα κατά το frame αυτό.
void test_state_update() {
	State state = state_create();
	TEST_ASSERT(state != NULL && state_info(state) != NULL);

	// Πληροφορίες για τα πλήκτρα (αρχικά κανένα δεν είναι πατημένο)
	struct key_state keys = { false, false, false, false, false, false, false };
	
	//test για οριζόντια κίνηση μπάλας
	// Χωρίς κανένα πλήκτρο, η μπάλα μετακινείται 4 pixels δεξιά
	Rectangle old_rect = state_info(state)->ball->rect;			
	state_update(state, &keys);
	Rectangle new_rect = state_info(state)->ball->rect;

	TEST_ASSERT( new_rect.x == old_rect.x + 4 );

	// Με πατημένο το δεξί βέλος, η μπάλα μετακινείται 6 pixels δεξιά
	keys.right = true;
	old_rect = state_info(state)->ball->rect;
	state_update(state, &keys);
	new_rect = state_info(state)->ball->rect;

	TEST_CHECK( new_rect.x == old_rect.x + 6 );

	// Με πατημένο το αεριστερό βέλος, η μπάλα μετακινείται 1 pixel δεξιά
	keys.right = false;
	keys.left = true;
	old_rect = state_info(state)->ball->rect;
	state_update(state, &keys);
	new_rect = state_info(state)->ball->rect;

	TEST_CHECK( new_rect.x == old_rect.x + 1 );

	//test για κατακόρυφη κίνηση μπάλας
	// Όταν η μπα΄λα βρίσκεται σε ηρεμία με πατημένο το άνω βέλος 
	state_info(state)->ball->vert_mov = IDLE;				
	keys.up = true;
	state_update(state, &keys);
	TEST_ASSERT(state_info(state)->ball->vert_speed == 17);  	// H αρχική ταχύτητα γίνεται 17.
	TEST_ASSERT(state_info(state)->ball->vert_mov == JUMPING);	// Kaι μπαίνει σε κατάσταση άλματος

	// Όταν η μπάλα πηδάει
	state_info(state)->ball->vert_mov = JUMPING;
	old_rect = state_info(state)->ball->rect;	// Η παλιά θέση της μπάλας
	float old_vert_speed =  state_info(state)->ball->vert_speed;	// Η κατακόρυφη ταχύτητά της
	state_update(state, &keys);
	new_rect = state_info(state)->ball->rect;	// Η νέα θέση της μπάλας
	TEST_CHECK(new_rect.y == old_rect.y - old_vert_speed);	// Μετακινείται προς τα πάνω τόσα pixels όσα η κατακόρυφη ταχύτητά της
	

	// Όταν η μπάλα πέφτει
	state_info(state)->ball->vert_mov = FALLING;
	old_rect = state_info(state)->ball->rect;	// Η παλιά θέση της μπάλας
	old_vert_speed =  state_info(state)->ball->vert_speed;
	state_update(state, &keys);
	new_rect = state_info(state)->ball->rect;	// Η νέα θέση της μπάλας
	TEST_CHECK(new_rect.y == old_rect.y + old_vert_speed);	// Μετακινείται προς τα κάτω τόσα pixels όσα η κατακόρυφη ταχύτητά της


	// Όταν η μπάλα βρίσκεται σε κατακόρυφη ηρεμία
	state_info(state)->ball->vert_mov = IDLE;
	bool flag = false;
	List objs = state_objects(state, 0, 20 * SCREEN_WIDTH); // Όλα τα αντικείμενα πριν γίνει η επέκταση της πίστας
	
	for(ListNode node = list_first(objs) ; node != LIST_EOF ; node = list_next(objs, node)){  //Διάσχιση των αντικειμένων
		
		Object cur_obj = list_node_value(objs, node);   
		if(cur_obj->type == PLATFORM){ 					// Αν το αντικείμενο είναι πλατφόρμα
			state_info(state)->ball->vert_mov = IDLE;	// Βάζω τη μπάλα σε κατάσταση ηρεμίας
			keys.up = false;							// Γι αυτό και το πάνω πλήκτρο δεν πρέπει να είναι πατημένο
			state_update(state, &keys);
			if(state_info(state)->ball->rect.x >= cur_obj->rect.x  // Αν η μπάλα είναι στα όρια της πλατφόρμας μετά το update
			&& state_info(state)->ball->rect.x <= cur_obj->rect.x + cur_obj->rect.width ){
			flag= true;	// Αν βρεθεί έστω και μία πλατφόρμα δεν χρειάζεται να ελεγχθούν οι άλλες
				TEST_CHECK(state_info(state)->ball->rect.y == cur_obj->rect.y - 40);  // Η μπάλα πρέπει να ακολουθεί σε ύψος την πλατφόρμα
																	//(- 40=obj->rect.height επειδή βρίσκεται πάνω στην πλατφόρμα)
			}
		}
	}
	
	if(!flag){										// Αν δεν βρίσκεται στα όρια καμίας πλατφόρμας
		state_update(state, &keys);
		TEST_ASSERT(state_info(state)->ball->vert_mov == FALLING);	// Αρχίζει να πέφτει
	}

	
	//Συγκρούσεις
	// Όταν η μπάλα συγκρούεται με το κάτω μέρος της οθόνης
	state_info(state)->ball->vert_mov = FALLING;		
	state_info(state)->ball->rect.y = SCREEN_HEIGHT;	// Όταν το y της μπάλας ισούται ακριβώς με SCREEN_HEIGHT
	state_update(state, &keys);
	TEST_CHECK(state_info(state)->playing == false);	// Το παιχνίδι πρέπει να σταματάει

	state_info(state)->ball->rect.y = SCREEN_HEIGHT + 10;	// Όταν το y της μπάλας είναι μεγαλύτερο από SCREEN_HEIGHT
	state_update(state, &keys);
	TEST_CHECK(state_info(state)->playing == false);	// Το παιχνίδι πρέπει να σταματάει

	
	// Σύγκρουση μπάλας με πλατφόρμα
	for(ListNode node = list_first(objs) ; node != LIST_EOF ; node = list_next(objs, node)){ // Διάσχιση των αντικειμένων
        Object obj = list_node_value(objs, node);
		// Αν είναι πλατφόρμα και η μπάλα βρίσκεται πάνω απο την πλατφόρμα (το + 30 εξηγείται στη state.c)
		if (obj->type == PLATFORM && state_info(state)->ball->rect.y + 30 < obj->rect.y){ 
			state_info(state)->ball->vert_mov = FALLING;	 // Η μπάλα πρέπει να πέφτει
			state_info(state)->ball->rect = obj->rect;		 // Έτσι δημιουργώ τη σύγκρουση
			state_update(state, &keys);
			TEST_ASSERT(state_info(state)->ball->vert_mov == IDLE); // Μετά τη σύγκρουση η μπάλα πρέπει να μπει σε κατάσταση ηρεμίας
        }

    }
	
	
	objs = state_objects(state, 0, 20*SCREEN_WIDTH);	// Όλα τα αντικείμενα πριν γίνει η επέκταση της πίστας
	for(ListNode node=list_first(objs) ; node!=LIST_EOF ; node=list_next(objs, node)){
		Object cur_obj = list_node_value(objs, node);
		int old_size = list_size(objs);
		int new_size;
		// int old_score = state_info(state)->score;		
		// int new_score;

		// Σύγκρουση μπάλας με αστέρι
		if(cur_obj->type == STAR){ 
			state_info(state)->ball->rect = cur_obj->rect;	// Έτσι δημιουργώ τη σύγκρουση μπάλας με αστέρι
			state_update(state, &keys);
			objs = state_objects(state, 0, SCREEN_WIDTH*20);	// Όλα τα αντικείμενα μετά τη σ΄ύγκρουση
			// new_score = state_info(state)->score;
			new_size = list_size(objs);
			// TEST_CHECK(old_size == new_size + 1);			// Πρέπει να είναι η λίστα των αντικειμένων κατά ένα μικρότερη
																// αφού αφαιρέθηκε ένα αντικείμενο
			// TEST_CHECK(old_score == new_score - 10);			// Το σκορ μετά τη σύγκρουση πρέπει να αυξηθεί κατά 10
		}
		// Σύγκρουση πλατφόρμας με το κάτω μέρος της οθόνης
		else if(cur_obj->type == PLATFORM ){
			cur_obj->rect.y = SCREEN_HEIGHT;	// Δημιουργω τη σύγκρουση με το κάτω μέρος της οθόνης
			cur_obj->vert_mov = FALLING;		// Η πλατφόρμα πρέπει να είναι ασταθης και να βρίσκεται σε πτώση
			state_update(state, &keys);
			objs = state_objects(state, 0, SCREEN_WIDTH*20);	// Όλα τα αντικείμενα μετά τη σ΄ύγκρουση
			new_size = list_size(objs);
			TEST_CHECK(old_size == new_size + 1);	// Πρέπει να είναι η λίστα των αντικειμένων κατά ένα μικρότερη
													// αφού αφαιρέθηκε ένα αντικείμενο
		}
	}
}


void  test_set_utils(){
    
    Set set = set_create(compare_ints, NULL);	// Φτιάχνω ένα set και βάζω 3 ακέραιους
    set_insert(set,create_ints(1));
    set_insert(set,create_ints(5));
    set_insert(set,create_ints(7));
    Pointer find = set_find_eq_or_greater(set,create_ints(1)); // Δοκιμάζω έναν αριθμό που υπάρχει στο set
    TEST_CHECK(*(int*)find == 1);							   // Και ελέγχω αν τον βρίσκει
    find = set_find_eq_or_greater(set,create_ints(3));		   // Δοκιμάζω έναν αριθμό που δεν υπάρχει στο set
    TEST_CHECK(*(int*)find == 5);	// Και ελέγχω αν βρίσκει την μικρότερη τιμή του set που είναι μεγαλύτερη από τον αριθμό
    find = set_find_eq_or_greater(set,create_ints(8));	// Δοκιμάζω έναν αριθμό που δεν υπάρχει στο set και δεν υπάρχει μεγαλύτερος
    TEST_CHECK(find == NULL);		// Τότε πρέπει να επιστρέφει NULL

 

    find = set_find_eq_or_smaller(set,create_ints(1));	// Δοκιμάζω έναν αριθμό που υπάρχει στο set
    TEST_CHECK(*(int*)find == 1);						// Και ελέγχω αν τον βρίσκει
    find = set_find_eq_or_smaller(set,create_ints(6));  // Δοκιμάζω έναν αριθμό που δεν υπάρχει στο set
    TEST_CHECK(*(int*)find == 5);	// Και ελέγχω αν βρίσκει την μεγαλύτερη τιμή του set που είναι μικρότερη από τον αριθμό
    find = set_find_eq_or_smaller(set,create_ints(0));	// Δοκιμάζω έναν αριθμό που δεν υπάρχει στο set και δεν υπάρχει μικρότερος
    TEST_CHECK(find == NULL);		// Τότε πρέπει να επιστρέφει NULL
}

// Λίστα με όλα τα tests προς εκτέλεση
TEST_LIST = {
	{ "test_state_create", test_state_create },
	{ "test_state_update", test_state_update },
	{ "test_set_utils", test_set_utils},

	{ NULL, NULL } // τερματίζουμε τη λίστα με NULL
};