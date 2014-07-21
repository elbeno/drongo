BEGIN { curlin = 0; }


/^# [0-9]+ \".*\"/ {
  curf=$3;
  gsub(/\"/, "", curf);
  foff=$2-1;
  next;
}

{ 
  curlin++; foff++;
  if (curlin==lookingfor) {
    printf("%s:%d: uber error\n*** PREPROCESSED VERSION: \n%s\n", curf, foff, $0);
  }
}
