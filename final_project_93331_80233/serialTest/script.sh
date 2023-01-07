
img(){
    gcc -Wall genImg.c -o gen
    for i in {1..99}
    do    
        ./gen images/img$i.raw
    done
    rm gen
}



folder="./images"

if test -d "$folder"; then
  rm -rf $folder
  mkdir $folder
  img
else
  mkdir $folder
  img
fi
