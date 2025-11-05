#include "card_manager.h"

CardManager::CardManager() : cardCount(0), initialized(false) {
    for (int i = 0; i < MAX_AUTHORIZED_CARDS; i++) {
        authorizedCards[i] = "";
    }
}

bool CardManager::begin() {
    if (!SPIFFS.begin(true)) {
        #if DEBUG_SERIAL
        Serial.println("SPIFFS error");
        #endif
        initialized = false;
        return false;
    }
    
    #if DEBUG_SERIAL
    Serial.println("SPIFFS OK");
    #endif
    
    if (loadCardsFromSPIFFS()) {
        #if DEBUG_SERIAL
        Serial.print("Cards: ");
        Serial.println(cardCount);
        #endif
    } else {
        cardCount = 0;
    }
    
    initialized = true;
    return true;
}

bool CardManager::loadCardsFromSPIFFS() {
    if (!SPIFFS.exists("/cards.txt")) {
        return false;
    }
    
    File file = SPIFFS.open("/cards.txt", "r");
    if (!file) {
        #if DEBUG_SERIAL
        Serial.println("Error abrir cards.txt");
        #endif
        return false;
    }
    
    cardCount = 0;
    while (file.available() && cardCount < MAX_AUTHORIZED_CARDS) {
        String line = file.readStringUntil('\n');
        line.trim();
        if (line.length() > 0) {
            authorizedCards[cardCount] = line;
            cardCount++;
        }
    }
    
    file.close();
    return true;
}

bool CardManager::saveCardsToSPIFFS() {
    File file = SPIFFS.open("/cards.txt", "w");
    if (!file) {
        #if DEBUG_SERIAL
        Serial.println("Error escribir cards.txt");
        #endif
        return false;
    }
    
    for (int i = 0; i < cardCount; i++) {
        file.println(authorizedCards[i]);
    }
    
    file.close();
    return true;
}

int CardManager::findCardIndex(String uid) {
    uid.toUpperCase();
    for (int i = 0; i < cardCount; i++) {
        if (authorizedCards[i] == uid) {
            return i;
        }
    }
    return -1;
}

bool CardManager::addCard(String uid) {
    uid.toUpperCase();
    uid.trim();
    
    if (uid.length() == 0) {
        #if DEBUG_SERIAL
        Serial.println("UID vacío");
        #endif
        return false;
    }
    
    if (findCardIndex(uid) >= 0) {
        #if DEBUG_SERIAL
        Serial.println("Ya existe");
        #endif
        return false;
    }
    
    if (cardCount >= MAX_AUTHORIZED_CARDS) {
        #if DEBUG_SERIAL
        Serial.println("Max cards");
        #endif
        return false;
    }
    
    authorizedCards[cardCount] = uid;
    cardCount++;
    
    if (saveCardsToSPIFFS()) {
        #if DEBUG_SERIAL
        Serial.print("Added: ");
        Serial.println(uid);
        #endif
        return true;
    }
    
    return false;
}

bool CardManager::removeCard(String uid) {
    uid.toUpperCase();
    int index = findCardIndex(uid);
    
    if (index < 0) {
        #if DEBUG_SERIAL
        Serial.println("No encontrada");
        #endif
        return false;
    }
    
    // Mover todas las tarjetas después del índice hacia arriba
    for (int i = index; i < cardCount - 1; i++) {
        authorizedCards[i] = authorizedCards[i + 1];
    }
    cardCount--;
    authorizedCards[cardCount] = "";
    
    if (saveCardsToSPIFFS()) {
        #if DEBUG_SERIAL
        Serial.print("Removed: ");
        Serial.println(uid);
        #endif
        return true;
    }
    
    return false;
}

bool CardManager::isCardAuthorized(String uid) {
    uid.toUpperCase();
    return findCardIndex(uid) >= 0;
}

void CardManager::listCards() {
    if (cardCount == 0) {
        Serial.println("No cards");
        return;
    }
    
    Serial.println("=== Cards ===");
    for (int i = 0; i < cardCount; i++) {
        Serial.print(i + 1);
        Serial.print(": ");
        Serial.println(authorizedCards[i]);
    }
    Serial.print("Total: ");
    Serial.println(cardCount);
}

void CardManager::clearAllCards() {
    cardCount = 0;
    for (int i = 0; i < MAX_AUTHORIZED_CARDS; i++) {
        authorizedCards[i] = "";
    }
    saveCardsToSPIFFS();
    #if DEBUG_SERIAL
    Serial.println("All cards cleared");
    #endif
}

String CardManager::getCardAt(int index) {
    if (index >= 0 && index < cardCount) {
        return authorizedCards[index];
    }
    return "";
}

