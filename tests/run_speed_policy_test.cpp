#include "../src/run_speed_policy.h"
#include <cassert>
#include <cmath>
#include <cstdio>
#include <limits>
using namespace runspeed;
static bool eq(float a, float b) { return std::fabs(a - b) < 0.00001f; }
int main() {
    int tests=0;
    auto check=[&](bool b){ assert(b); ++tests; };
    check(ClampPercent(-10)==70); check(ClampPercent(70)==70);
    check(ClampPercent(90)==90); check(ClampPercent(100)==100); check(ClampPercent(150)==100);
    check(ShouldScale(true,true,false,3,1,1.f,0.f));
    check(ShouldScale(true,true,false,4,2,1.f,0.f));
    check(!ShouldScale(true,true,false,2,0,1.f,0.f));
    check(!ShouldScale(true,true,false,1,3,1.f,0.f));
    check(!ShouldScale(false,true,false,3,1,1.f,0.f));
    check(!ShouldScale(true,false,false,3,1,1.f,0.f));
    check(!ShouldScale(true,true,true,3,1,1.f,0.f));
    check(!ShouldScale(true,true,false,3,44,1.f,0.f));
    check(!ShouldScale(true,true,false,3,1,0.02f,0.f));
    check(!ShouldScale(true,true,false,3,1,1.f,-2.f));
    RateController ctl; int p=1, a=2, h=3;
    ctl.BeginFrame();
    float first=ctl.Update(&p,&a,&h,1,1.f,true,90);
    check(eq(first,.9f));
    check(eq(ctl.Update(&p,&a,&h,1,first,true,90),.9f)); // no exponential slowdown
    check(eq(ctl.Update(&p,&a,&h,1,.9f,true,80),.8f)); // setting changes use native
    check(eq(ctl.Update(&p,&a,&h,1,.8f,false,90),1.f)); // return to walk restores speed
    ctl.EndFrame();
    ctl.BeginFrame();
    check(eq(ctl.Update(&p,&a,&h,1,1.2f,true,90),1.08f)); // engine changed baseline
    check(eq(ctl.Update(&p,&a,&h,1,1.08f,true,90),1.08f));
    check(eq(ctl.Update(&p,&a,&h,1,1.08f,false,90),1.2f));
    check(eq(ctl.Update(&p,&a,&h,1,1.2f,true,100),1.2f)); // disable
    ctl.EndFrame();
    ctl.BeginFrame();
    check(eq(ctl.Update(&p,&a,&h,2,1.f,true,90),.9f)); // new anim id
    check(eq(ctl.Update(&p,&a,&h,2,.9f,true,90),.9f));
    check(eq(ctl.Update(&p,&a,&h,2,.9f,true,70),.7f));
    check(eq(ctl.Update(&p,&a,&h,2,.7f,true,200),1.f));
    float nan=std::numeric_limits<float>::quiet_NaN();
    check(std::isnan(ctl.Update(&p,&a,&h,2,nan,true,90))); // invalid ignored
    check(eq(ctl.Update(&p,&a,&h,2,-2.f,true,90),-2.f));
    ctl.EndFrame();
    ctl.BeginFrame(); // cached association dropped while animation no longer active
    ctl.EndFrame();
    check(eq(ctl.Update(&p,&a,&h,1,1.f,true,90),.9f));
    check(eq(ctl.Update(&p,&a,&h,1,.9f,false,90),1.f));
    ctl.EndFrame();
    std::printf("Run-speed policy: %d checks PASS\n", tests);
}
