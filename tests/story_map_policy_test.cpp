#include "../src/story_map_policy.h"
#include <cassert>
#include <cstdio>
int main() {
    using namespace story_map;
    assert(IsContact(0x02A7) && IsContact(0x0570));
    assert(!IsContact(0x018A) && !IsContact(0x02A8) && !IsContact(0x0164));
    Key a=Make(123.0f,-232.5f,5.0f,18), b=Make(123.1f,-232.4f,5.5f,18);
    Key c=Make(400.0f,-232.5f,5.0f,18), d=Make(123.0f,-232.5f,5.0f,22);
    assert(Same(a,a) && Same(a,b) && !Same(a,c) && !Same(a,d));
    Key list[]={a,c};
    assert(Included(a,list,2) && Included(b,list,2) && !Included(d,list,2));
    assert(!Included(a,list,0));
    assert(ValidCount(MAX_CONTACTS) && !ValidCount(MAX_CONTACTS+1));
    assert(sizeof(Key)==16 && PACKET_SIZE <= 480);
    puts("story map policy: 14 checks PASS");
    return 0;
}
