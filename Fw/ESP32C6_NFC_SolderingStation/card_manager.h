#ifndef CARD_MANAGER_H
#define CARD_MANAGER_H

#include <Arduino.h>
#include <FS.h>
#include <SPIFFS.h>
#include "config.h"

class CardManager {
private:
    String authorizedCards[MAX_AUTHORIZED_CARDS];
    int cardCount;
    bool initialized;
    
    bool loadCardsFromSPIFFS();
    bool saveCardsToSPIFFS();
    int findCardIndex(String uid);
    
public:
    CardManager();
    bool begin();
    bool addCard(String uid);
    bool removeCard(String uid);
    bool isCardAuthorized(String uid);
    void listCards();
    void clearAllCards();
    int getCardCount() { return cardCount; }
    String getCardAt(int index);
};

#endif // CARD_MANAGER_H


