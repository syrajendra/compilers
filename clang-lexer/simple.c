struct dummy {
  int x;
  char y;
};

float func(int a) 
{
  float b = a + 10;
  return b;
}

int main()
{
 	  const int k = 5;
  	int *kptr = &k;
  	*kptr = 7;
  	return 0;
}
