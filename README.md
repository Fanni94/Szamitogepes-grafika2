**Számítógépes grafika beadandó feladat**

- Legyen P(u,v,d) = (1+d)*[ cos(2pi*u)*sin(pi*v), sin(2pi*u)*sin(pi*v), cos(pi*v)] a parametrikus felület alakja (gömb). A magasságtérkép legyen véletlenszerű zaj, azaz d=m(u,v)= -0.05 és 0.05 közti random érték. A véletlenszámokat a program elején egyszer generáljuk (rácspontokban), majd (az egyszerűség kedvéért) tároljuk uniform tömbben.

- A színtérnek megfelelő szabadon választott textúrát lehet alkalmazni, akár a netről is le lehet szedni, de legalább két különböző fájlból olvassuk be, és értelemszerűen külön textúraként használjuk! Legyen áttűnés a két textúra között, azaz 2 mp alatt menjen át egyik textúrából a másikba, majd ugyanannyi idő alatt vissza.

- Lehessen a UI-on fényforrásokat hozzáadni egy listához (max 10-et). A fényforrások pozíciója és színe legyen állítható. Lehet minden fényforrás azonos típusú (irány, pont, stb.), olyan, amilyen a második részfeladatban elvárt (ha ott nincs megkötve, itt is tetszőleges).

- Az egérrel a kamera eye vektora forogjon az at vektor körül, úgy mint az at az eye körül (CAD/ geometria szerkesztőhöz hasonló kameramozgás).