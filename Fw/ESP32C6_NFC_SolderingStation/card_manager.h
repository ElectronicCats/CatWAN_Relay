#ifndef CARD_MANAGER_H
#define CARD_MANAGER_H

#include <Arduino.h>
#include <FS.h>
#include <SPIFFS.h>
#include "config.h"

class CardManager {
private:
    String authorizedCards[MAX_AUTHORIZED_CARDS];
    int cardRelays[MAX_AUTHORIZED_CARDS]; // Asignación de relé para cada tarjeta (-1 = no asignado)
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
    
    // Funciones para asignación tarjeta-relé
    bool assignCardToRelay(String uid, int relayIndex);
    int getCardRelay(String uid); // Retorna el índice del relé asignado (-1 si no tiene asignación)
    void listAssignments(); // Listar todas las asignaciones
};

#endif // CARD_MANAGER_H