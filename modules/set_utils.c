#include <stdlib.h>

#include "ADTSet.h"
#include "ADTList.h"
#include "state.h"
#include "set_utils.h"
#include "common_types.h"


Pointer set_find_eq_or_greater(Set set, Pointer value){
    SetNode node = set_find_node(set, value);
    if(node != NULL){                       // Αν βρεθεί ο κόμβος που περιέχει το value
        return set_node_value(set, node);   // Επιστρέφει την μοναδική τιμή του set που είναι ισοδύναμη με value
    }
    set_insert(set, value);     // Αλλιώς τοποθετε΄΄ι στο set το value, το οποίο μπαίνει αυτόματα στη κατάλληλη θέση
    node = set_find_node(set, value);   // Βρίσει σε ποιο κόμβο είναι
    SetNode next_node = set_next(set, node);    //  Και παίρνει τον επόμενο
    set_remove(set, value);    // Αφού βρήκαμε το στοιχείο που θέλουμε αφαιρούμε το στοιχείο που προσθέσαμε
    if(next_node!=SET_EOF){    // Αν ο επόμενος κόμβος δεν είναι EOF
        return set_node_value(set, next_node); // Τότε επιστρέφει το value του
    }
    return NULL;    // Αν ο επόμενος κόμβος είναι EOF τότε δεν υπάρχει μεγαλύτερο στοιχείο
}

 
Pointer set_find_eq_or_smaller(Set set, Pointer value){
    SetNode node = set_find_node(set, value);
    if(node != NULL){                       // Αν βρεθεί ο κόμβος που περιέχει το value
        return set_node_value(set, node);   // Επιστρέφει την μοναδική τιμή του set που είναι ισοδύναμη με value
    }
    set_insert(set, value);         // Αλλιώς τοποθετε΄΄ι στο set το value, το οποίο μπαίνει αυτόματα στη κατάλληλη θέση
    node = set_find_node(set, value);   // Βρίσει σε ποιο κόμβο είναι
    SetNode prev_node = set_previous(set, node);    //  Και παίρνει τον προηγούμενο
    set_remove(set, value);     // Αφού βρήκαμε το στοιχείο που θέλουμε αφαιρούμε το στοιχείο που προσθέσαμε
    if(prev_node!=SET_BOF){     // Αν ο επόμενος κόμβος δεν είναι BOF
        return set_node_value(set, prev_node);      // Τότε επιστρέφει το value του
    }
    return NULL;        // Αν ο επόμενος κόμβος είναι BOF τότε δεν υπάρχει μικρότερο στοιχείο
}