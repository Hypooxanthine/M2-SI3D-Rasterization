# Rapport du projet lancer de rayons

## Ressources

[Sources du projet](https://github.com/Hypooxanthine/M2-SI3D-Rasterization/tree/master/TP2_SI3D)

Pour compiler et lancer, depuis le répertoire racine de gkit : 
```shell
premake4 gmake
./run2.sh
```

## Screenshots

![Screenshot](screenshots/rendu512.png)

## Travail réalisé

Le travail que j'ai réalisé a suivi l'énoncé du TP.

La fonction Lr permet d'obtenir la couleur pour un point donné. Le mode d'éclairage est direct uniquement, c'est à dire que pour chaque point, je vais aller vérifier quel est l'éclairage direct total. Donc, pour 1 point, je cherche tous les triangles émissifs de la scène. Ensuite, pour chaque triangle émissif, je tire un certain nombre de rayons à l'intérieur, grâce à 2 variables aléatoires, et je divise par la pdf. Je peux ensuite injecter cela dans l'estimateur de Monte-Carlo.
La couleur retournée pour le point est utilisée pour donner une couleur à chaque pixel. Le résultat pour 512 rayons lancés par triangle émissif est visible dans le screenshot ci-dessus.
