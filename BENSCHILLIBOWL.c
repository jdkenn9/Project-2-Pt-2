#include "BENSCHILLIBOWL.h"

#include <assert.h>
#include <stdlib.h>
#include <time.h>

bool IsEmpty(BENSCHILLIBOWL* bcb);
bool IsFull(BENSCHILLIBOWL* bcb);
void AddOrderToBack(Order **orders, Order *order);

MenuItem BENSCHILLIBOWLMenu[] = { 
    "BensChilli", 
    "BensHalfSmoke", 
    "BensHotDog", 
    "BensChilliCheeseFries", 
    "BensShake",
    "BensHotCakes",
    "BensCake",
    "BensHamburger",
    "BensVeggieBurger",
    "BensOnionRings",
};
int BENSCHILLIBOWLMenuLength = 10;

/* Select a random item from the Menu and return it */
MenuItem PickRandomMenuItem() {
    return BENSCHILLIBOWLMenu[(rand() % BENSCHILLIBOWLMenuLength)];
}

/* Allocate memory for the Restaurant, then create the mutex and condition variables needed to instantiate the Restaurant */

BENSCHILLIBOWL* OpenRestaurant(int max_size, int expected_num_orders) {
  BENSCHILLIBOWL* open = (BENSCHILLIBOWL*)malloc(sizeof(BENSCHILLIBOWL));

  open->orders = NULL;
  open->current_size = 0;
  open->max_size = max_size;
  open->next_order_number = 1;
  open->orders_handled = 0;
  open->expected_num_orders = expected_num_orders;

  pthread_mutex_init(&(open->mutex), NULL);
  pthread_cond_init(&(open->can_add_orders), NULL);
  pthread_cond_init(&(open->can_get_orders), NULL);

  printf("Restaurant is open!\n");
  return open;
}


/* check that the number of orders received is equal to the number handled (ie.fullfilled). Remember to deallocate your resources */

void CloseRestaurant(BENSCHILLIBOWL* bcb) {
  if (bcb->expected_num_orders != bcb->orders_handled){
    printf("orders not finished!");
    exit(0);
  }
  printf("Restaurant is closed!\n");
  pthread_mutex_destroy(&(bcb->mutex));
  pthread_cond_destroy(&(bcb->can_add_orders));
  pthread_cond_destroy(&(bcb->can_get_orders));
  free(bcb);
  
}

/* add an order to the back of queue */
int AddOrder(BENSCHILLIBOWL* bcb, Order* order) {
    pthread_mutex_lock(&(bcb->mutex));
    while(IsFull(bcb)){
      pthread_cond_wait(&(bcb->can_add_orders), &(bcb->mutex));
    }
    if(bcb->orders == NULL){
      bcb->orders = order;
      order->next = NULL;
    }
    else{
      AddOrderToBack(&bcb->orders, order);
    }
    order->order_number = bcb->next_order_number;
    bcb->next_order_number += 1;
    pthread_cond_broadcast(&(bcb->can_get_orders));
    pthread_mutex_unlock(&(bcb->mutex));
    return order->order_number;
}

/* remove an order from the queue */
Order *GetOrder(BENSCHILLIBOWL* bcb) {
  pthread_mutex_lock(&(bcb->mutex));
  while(IsEmpty(bcb)){
    if (bcb->orders_handled == bcb->expected_num_orders){
      pthread_mutex_unlock(&(bcb->mutex));
      return NULL;
    }
    pthread_cond_wait(&(bcb->can_get_orders), &(bcb->mutex));
  }
  Order* order = bcb->orders;
  bcb->orders = order->next;
  bcb->current_size -= 1;
  bcb->orders_handled += 1;
  pthread_cond_broadcast(&(bcb->can_add_orders));
  pthread_mutex_unlock(&(bcb->mutex));
  return order;
}

// Optional helper functions (you can implement if you think they would be useful)
bool IsEmpty(BENSCHILLIBOWL* bcb) {
  return (bcb->orders == NULL);
}

bool IsFull(BENSCHILLIBOWL* bcb) {
  int count = 0;
  int max = bcb->max_size;
  Order* current = bcb->orders;
  
  while(current != NULL){
    count+=1;
    current = current->next;
  }
  return (max == count);
}

/* this methods adds order to rear of queue */
void AddOrderToBack(Order **orders, Order *order) {
  if(*orders == NULL){
    *orders = order;
  }
  else{
    Order* current = *orders;
    while(current->next){
      current = current->next;
    }
    current->next = order;
  }
}

