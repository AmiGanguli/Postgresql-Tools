

int main()
{
  void* pParser = ParseAlloc (malloc);

  /* First input: 
      15 / 5
                                */
  Parse (pParser, TK_INTEGER, 15);
  Parse (pParser, TK_SLASH, 0);
  Parse (pParser, TK_INTEGER, 5);
  Parse (pParser, 0, 0);

  /*  Second input:
        50 + 125
                               */


  Parse (pParser, TK_INTEGER, 50);
  Parse (pParser, TK_PLUS, 0);
  Parse (pParser, TK_INTEGER, 125);
  Parse (pParser, 0, 0);


  /*  Third input:
        50 * 125 + 125
                               */



  Parse (pParser, TK_INTEGER, 50);
  Parse (pParser, TK_STAR, 0);
  Parse (pParser, TK_INTEGER, 125);
  Parse (pParser, TK_PLUS, 0);
  Parse (pParser, TK_INTEGER, 125);
  Parse (pParser, 0, 0);


  ParseFree(pParser, free );

}
