#include "card_manager.h"

CardManager::CardManager() : cardCount(0), initialized(false) {
    for (int i = 0; i < MAX_AUTHORIZED_CARDS; i++) {
        authorizedCards[i] = "";
        cardRelays[i] = -1; // -1 = no asignado
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
            // Formato: UID o UID:relé (ej: "04A5B6C7D8" o "04A5B6C7D8:0")
            int colonIndex = line.indexOf(':');
            if (colonIndex > 0) {
                // Formato con relé asignado
                authorizedCards[cardCount] = line.substring(0, colonIndex);
                authorizedCards[cardCount].toUpperCase();
                cardRelays[cardCount] = line.substring(colonIndex + 1).toInt();
            } else {
                // Formato sin asignación (solo UID)
                authorizedCards[cardCount] = line;
                authorizedCards[cardCount].toUpperCase();
                cardRelays[cardCount] = -1; // No asignado
            }
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
        // Guardar en formato: UID:relé (si tiene asignación) o solo UID (si no tiene)
        if (cardRelays[i] >= 0) {
            file.print(authorizedCards[i]);
            file.print(':');
            file.println(cardRelays[i]);
        } else {
            file.println(authorizedCards[i]);
        }
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
    cardRelays[cardCount] = -1; // Sin asignación por defecto
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
        cardRelays[i] = cardRelays[i + 1];
    }
    cardCount--;
    authorizedCards[cardCount] = "";
    cardRelays[cardCount] = -1;
    
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
        Serial.print(authorizedCards[i]);
        if (cardRelays[i] >= 0) {
            Serial.print(" -> Relé ");
            Serial.print(cardRelays[i] + 1);
        } else {
            Serial.print(" -> Sin asignación");
        }
        Serial.println();
    }
    Serial.print("Total: ");
    Serial.println(cardCount);
}

void CardManager::clearAllCards() {
    cardCount = 0;
    for (int i = 0; i < MAX_AUTHORIZED_CARDS; i++) {
        authorizedCards[i] = "";
        cardRelays[i] = -1;
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

bool CardManager::assignCardToRelay(String uid, int relayIndex) {
    uid.toUpperCase();
    int index = findCardIndex(uid);
    
    if (index < 0) {
        #if DEBUG_SERIAL
        Serial.println("Tarjeta no encontrada");
        #endif
        return false;
    }
    
    if (relayIndex < 0 || relayIndex >= MAX_RELAYS) {
        #if DEBUG_SERIAL
        Serial.println("Relé inválido (0-");
        Serial.print(MAX_RELAYS - 1);
        Serial.println(")");
        #endif
        return false;
    }
    
    cardRelays[index] = relayIndex;
    
    if (saveCardsToSPIFFS()) {
        #if DEBUG_SERIAL
        Serial.print("Tarjeta ");
        Serial.print(uid);
        Serial.print(" asignada a Relé ");
        Serial.println(relayIndex + 1);
        #endif
        return true;
    }
    
    return false;
}

int CardManager::getCardRelay(String uid) {
    uid.toUpperCase();
    int index = findCardIndex(uid);
    
    if (index < 0) {
        return -1; // Tarjeta no encontrada
    }
    
    return cardRelays[index]; // Retorna el índice del relé (-1 si no tiene asignación)
}

void CardManager::listAssignments() {
    if (cardCount == 0) {
        Serial.println("No hay asignaciones");
        return;
    }
    
    Serial.println("=== Asignaciones ===");
    for (int i = 0; i < cardCount; i++) {
        if (cardRelays[i] >= 0) {
            Serial.print("Tarjeta: ");
            Serial.print(authorizedCards[i]);
            Serial.print(" -> Cautín ");
            Serial.println(cardRelays[i] + 1);
        }
    }
}