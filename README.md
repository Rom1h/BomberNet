
# **BomberNet - Jeu Bomberman Multijoueur en Réseau**

## **Présentation**
**BomberNet** est un projet universitaire implémenté en **C**, inspiré du célèbre jeu Bomberman. Ce projet permet à plusieurs joueurs de s’affronter via une communication client-serveur en réseau. Le jeu propose un mode solo et un mode équipe, avec des contrôles simples et une mécanique intuitive.

Le jeu met en œuvre des concepts avancés de **programmation réseau**, de **programmation concurrente**, et de gestion des interactions en temps réel.

---

## **Fonctionnalités**
- **Mode Solo et Équipe :** Jouez seul ou collaborez avec d'autres joueurs.
- **Communication réseau :** Utilisation des sockets pour les échanges entre client et serveur.
- **Commandes intuitives :** Contrôlez votre joueur avec des touches claviers simples.
- **Chat intégré :** Communiquez en direct avec les autres joueurs.
- **Gestion concurrente :** Support pour plusieurs joueurs connectés simultanément.
---

## **Compilation**
Pour compiler le projet (serveur et client) :
```bash
make all
```

---

## **Exécution**
### Serveur :
Lancer le serveur en spécifiant un port :
```bash
./serveur [PORT]
```

### Client :
Lancer un client en précisant les paramètres suivants :
```bash
./client [ADRESSE] [PORT] [MODE]
```
- **ADRESSE :** Adresse IP du serveur.
- **PORT :** Port utilisé pour la communication.
- **MODE :**
  - `1` : Partie solo.
  - `2` : Partie en équipe.

---

## **Commandes Clavier**
Pendant une partie, utilisez les touches suivantes :
- **Flèches directionnelles :** Déplacer le joueur.
- `*` : Poser une bombe.
- `&` : Annuler le dernier déplacement.
- `CTRL + C` : Quitter la partie.
- **Autres touches :** Écrire un message dans le chat (envoyez avec Entrée).

---

## **Technologies Utilisées**
- **Langage** : C
- **Réseau** : Sockets TCP/UDP pour la communication.
- **Protocole :** Gestion des données entre clients et serveur.
- **Concurrent :** Gestion des threads pour les connexions multiples.
- **Système d’exploitation** : Développé et testé sous Linux.

---

## **Apprentissage et Réalisations**
- Maîtrise de l’implémentation de sockets réseau en C.
- Programmation concurrente et gestion des threads.
- Développement d’un protocole de communication pour un jeu multijoueur.
- Gestion des interactions joueur-serveur en temps réel.
