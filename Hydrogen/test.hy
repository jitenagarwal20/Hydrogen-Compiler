assume x = 5;
assume y = 10;
incase(xy){
    x=x*2;
}
otherwise{
    x=x-2;
}
exit(x);