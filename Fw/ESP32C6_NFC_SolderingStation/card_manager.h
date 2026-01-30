#ifndef CARD_MANAGER_H
#define CARD_MANAGER_H

#include <Arduino.h>
#include <FS.h>
#include <SPIFFS.h>
#include "config.h"

#define UID_MAX_LEN 16
#define NAME_MAX_LEN 32

// Estructura que representa una tarjeta NFC
struct CardEntry {
    char uid[UID_MAX_LEN];      // UID de la tarjeta
    char name[NAME_MAX_LEN];      // Nombre del usuario
    int relayIndex;   // Relé asignado (-1 si no tiene)
};

class CardManager {
private:
    CardEntry cards[MAX_AUTHORIZED_CARDS];
    int cardCount;
    bool initialized;
    
    bool loadCardsFromSPIFFS();
    bool saveCardsToSPIFFS();
    int findCardIndex(const char* uid);
    
public:
    CardManager();
    bool begin();

    // Gestión de tarjetas
    bool addCard(String uid, String name);
    bool removeCard(String uid);
    bool isCardAuthorized(String uid);
    String getCardName(String uid);

    void listCards();
    void clearAllCards();

    int getCardCount() { return cardCount; }
    String getCardAt(int index);

    // Asignación tarjeta–relé
    bool assignCardToRelay(String uid, int relayIndex);
    int getCardRelay(String uid);
    void listAssignments();
};

#endif // CARD_MANAGER_H
