# Manuel d'aide de Simple Clicker

- Vous pouvez utiliser cette application pour automatiser vos clics de souris.
- AVERTISSEMENT : ce logiciel ne doit pas être utilisé comme outil de triche (auto-clic) dans les jeux. Si vous le faites, nous, en tant que développeur de cette application, ne sommes ABSOLUMENT PAS responsables. Consultez la licence concernant la garantie.

## Préréglages

- Vous pouvez avoir autant de préréglages que vous le souhaitez.
- Chaque préréglage possède son propre « jeu de paramètres ».
- Vous pouvez en utiliser un pour un usage précis et un autre pour un usage différent.
- Double-cliquez sur un préréglage dans le menu de gauche pour passer dessus.

### Création d'un préréglage
- Créez un nouveau préréglage en appuyant sur le bouton `+` à droite du titre `Préréglages`, en haut à gauche de la fenêtre.
- Une fois créé, vous pouvez directement lui donner un nom en le saisissant.
- Un nom du type `Nouveau préréglage N` est généré par défaut pour chaque préréglage créé.
- Lorsque vous avez fini de renommer, appuyez sur `ENTRÉE` ou sur le bouton d'enregistrement à droite du nom du préréglage pour valider le nouveau nom.
- Vous pouvez aussi annuler le renommage en appuyant sur `ÉCHAP` ou sur le bouton d'annulation à droite du bouton d'enregistrement.

### Enregistrer/annuler un préréglage
- Lorsqu'un paramètre est modifié sur la droite, le nom du préréglage passe en italique et en gras, et un astérisque apparaît à gauche du nom. Cela indique que le préréglage a été modifié.
- Cliquez sur l'icône d'enregistrement ou appuyez sur `Ctrl+S` au clavier pour enregistrer les modifications.
- Cliquez sur l'icône d'annulation ou appuyez sur `Ctrl+K` au clavier pour abandonner les modifications. (Rétablit l'état précédent)

### Gestion des préréglages
- Pour supprimer un préréglage, passez d'abord la souris sur ce préréglage : un bouton corbeille apparaît. Cliquez dessus pour supprimer le préréglage.
- Pour renommer à nouveau un préréglage, passez d'abord la souris dessus : une icône de crayon apparaît. Cliquez dessus, un champ de saisie s'affiche, saisissez le nouveau nom souhaité. Appuyez ensuite sur `ENTRÉE` ou cliquez sur le bouton d'enregistrement pour valider le renommage, OU appuyez sur `ÉCHAP` ou cliquez sur le bouton d'annulation pour l'annuler.
- Pendant un renommage, vous pouvez toujours modifier les paramètres du préréglage. Mais lorsque vous validez le renommage, le préréglage est également enregistré. Il en va de même pour l'annulation du renommage (les modifications sont annulées).

## Paramètres du préréglage
- Vous pouvez modifier les paramètres dans l'interface située à droite des préréglages.
- `Intervalle de clic` détermine le temps écoulé entre les clics. Cette valeur doit être supérieure à 0.
- `Durée de maintien` détermine combien de temps un bouton de la souris reste enfoncé. Cette valeur doit être supérieure à 0.
- `Options de clic` détermine les détails des clics de souris.
- `Bouton de la souris` est le bouton de souris utilisé pour cliquer. Vous pouvez choisir Gauche, Droite, Milieu, Précédent (latéral) et Suivant (latéral).
- `Mode de clic` détermine s'il faut appuyer puis relâcher, ou maintenir le bouton de la souris enfoncé.
- `Répéter` détermine combien de fois le clic est répété. Cette valeur doit être supérieure à 0. La limite est de 1 million.
- Lorsque `Répéter jusqu'à l'arrêt` est sélectionné, les clics continuent jusqu'à ce que vous appuyiez sur le raccourci pour les arrêter.
- `Position du curseur de la souris` est l'emplacement absolu sur votre écran où le clic est effectué. Si vous avez plusieurs écrans, cela peut ne pas fonctionner.
- Si vous choisissez `Actuelle`, les valeurs X et Y sont ignorées, car le clic est effectué là où se trouve votre curseur.
- Pour choisir un emplacement depuis l'interface, cliquez sur `Choisir` : l'écran sera figé et assombri pour que vous puissiez choisir un emplacement.
- Placez votre curseur à l'endroit souhaité et cliquez avec le bouton gauche de la souris pour choisir cet emplacement, ou appuyez sur `ÉCHAP` pour annuler.

## Paramètres
- Cliquez sur le bouton `Paramètres` en bas de la fenêtre pour ouvrir la fenêtre des paramètres.
- Vous pouvez y modifier le thème, la langue et le raccourci global.
- Langues prises en charge : anglais, turc, allemand et français.
- Thèmes prises en charge : sombre et clair.
- Pour modifier le raccourci, cliquez sur ce champ de texte puis appuyez sur une touche ou une combinaison de touches de votre clavier.
- Cliquez ensuite sur Enregistrer pour sauvegarder les paramètres, ou sur Annuler pour abandonner les modifications.

## Raccourci
- Le raccourci que vous définissez est écouté par l'application partout, tant que vous ne fermez pas l'application.
- Appuyez sur le raccourci pour activer ou désactiver le clicker.
- Ce comportement diffère sous Linux avec le compositeur Wayland.
- Wayland limite les raccourcis globaux au programme lui-même, mais ce n'est pas le cas dans KDE Plasma.
- Dans KDE, les raccourcis globaux sont possibles via XCB, car le programme s'exécute sous XWayland.
- Sous X11, les raccourcis globaux fonctionnent parfaitement, comme sous Windows.

## Mise à jour automatique
- Le programme vous informe au démarrage lorsqu'une nouvelle mise à jour est disponible.
- Une notification du type `Nouvelle mise à jour disponible` apparaît. Cliquez sur `Mettre à jour maintenant` pour mettre à jour l'application.
- Ce processus de mise à jour est entièrement automatique. Il demande uniquement les droits d'administrateur sous Windows et les privilèges root sous Linux.
- REMARQUE : ces privilèges sont nécessaires pour installer l'application. Si vous souhaitez voir tout le processus d'installation et les opérations effectuées en arrière-plan, le projet est open source.
- L'application s'ouvrira à la fin de la mise à jour.

# À propos

- Créé par [ilpeN](https://github.com/ilpenSE).
- Développé en C++ avec Qt.
- Licence : AGPL-3.0 uniquement (GNU Affero General Public License)
