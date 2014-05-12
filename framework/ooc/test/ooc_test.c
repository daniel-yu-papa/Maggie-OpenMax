#include "ooc_test.h"
#include "agilelog.h"
#include <stdlib.h>
#include <string.h>
#include "Mag_base.h"

/*Parent class: Polygon*/
AllocateClass(Polygon, Base);

/*int virtual_polygon_get_edges(Polygon thiz){
    AGILE_LOGD("[%s]: edges number is %d", __FUNCTION__, thiz->mEdgeNum);

    return 0;
}*/

int Polygon_setColor(Polygon thiz, int colour){
    thiz->mColor = colour;
    AGILE_LOGD("[%s]: set color %d", __FUNCTION__, colour);
    return 0;
}

int Polygon_getColor(Polygon thiz){
    AGILE_LOGD("[%s]: get color %d", __FUNCTION__, thiz->mColor);
    return thiz->mColor;
}

void Polygon_setOwner(Polygon thiz, char *owner){
    strcpy(thiz->mpOwner, owner);
    AGILE_LOGD("[%s]: set owner %s", __FUNCTION__, owner);
}

char *Polygon_getOwner(Polygon thiz){
    AGILE_LOGD("[%s]: get owner %s", __FUNCTION__, thiz->mpOwner);
    return thiz->mpOwner;
}

static void Polygon_initialize(Class this){
    PolygonVtableInstance.getEdges = NULL; //virtual_polygon_get_edges;
}

static void Polygon_constructor(Polygon thiz, const void *params){
    MAG_ASSERT(ooc_isInitialized(Polygon));
    chain_constructor(Polygon, thiz, params);
    thiz->setColor = Polygon_setColor;
    thiz->getColor = Polygon_getColor;
    thiz->setOwner = Polygon_setOwner;
    thiz->getOwner = Polygon_getOwner;

    thiz->mpOwner = (char *)malloc(16);
    thiz->mEdgeNum = *((int *)params + 0);
    thiz->mColor = *((int *)params + 1);
    AGILE_LOGD("[%s]: EdgeNum = %d, Color = %d", __FUNCTION__, thiz->mEdgeNum, thiz->mColor);
}

static void Polygon_destructor(Polygon thiz, PolygonVtable vtab){
    free(thiz->mpOwner);
    AGILE_LOGD("[%s]", __FUNCTION__);
}

/*Child class: Triangle*/
AllocateClass(Triangle, Polygon);

int virtual_triangle_sellToCustomer(Triangle thiz, int money){   
    thiz->mTotalPrice = thiz->mBasePrice + money;
    AGILE_LOGD("[%s]: total price = %d, base price = %d, money = %d", __FUNCTION__, 
                thiz->mTotalPrice, thiz->mBasePrice, money);
}


int virtual_triangle_get_edges(/*Polygon*/ void *thiz){
    Triangle self = ooc_cast(thiz, Triangle);
    AGILE_LOGD("[%s]: edges number is %d", __FUNCTION__, self->mEdgeNum);

    return 0;
}

int Triangle_bidBasePrice(Triangle thiz, int price){
    AGILE_LOGD("[%s]: bidding price is %d", __FUNCTION__, price);
    thiz->mBasePrice = price;
    return 0;
}

static void Triangle_initialize(Class this){
    TriangleVtableInstance.sellToCustomer = virtual_triangle_sellToCustomer;
    /*override the parent's virtual function*/
    TriangleVtableInstance.Polygon.getEdges = virtual_triangle_get_edges;
}

static void Triangle_constructor(Triangle thiz, const void *params){
    MAG_ASSERT(ooc_isInitialized(Triangle));
    chain_constructor(Triangle, thiz, params);

    thiz->bidBasePrice = Triangle_bidBasePrice;

    AGILE_LOGD("[%s]", __FUNCTION__);

    thiz->mEdgeNum = *((int *)params + 0);
}

static void Triangle_destructor(Triangle thiz, TriangleVtable vtab){
    AGILE_LOGD("[%s]", __FUNCTION__);
}

/*Child class: TriangleInGold*/
AllocateClass(TriangleInGold, Triangle);

int virtual_triangleInGold_sellToCustomer(Triangle thiz, int money){   
    TriangleInGold self = ooc_cast(thiz, TriangleInGold);
    self->mTotalPrice = self->mBasePrice + money * 10;
    AGILE_LOGD("[%s]: total price = %d, base price = %d, money = %d", __FUNCTION__, 
                self->mTotalPrice, self->mBasePrice, money);
}

static void TriangleInGold_initialize(Class this){
    /*override the parent's virtual function*/
    TriangleInGoldVtableInstance.Triangle.sellToCustomer = virtual_triangleInGold_sellToCustomer;
}

int TriangleInGold_bidBasePrice(TriangleInGold thiz, int price){
    AGILE_LOGD("[%s]: bidding price is %d", __FUNCTION__, price);
    thiz->mBasePrice = price;
    return 0;
}

static void TriangleInGold_constructor(TriangleInGold thiz, const void *params){
    MAG_ASSERT(ooc_isInitialized(TriangleInGold));
    chain_constructor(TriangleInGold, thiz, params);

    thiz->bidBasePrice = TriangleInGold_bidBasePrice;

    AGILE_LOGD("[%s]", __FUNCTION__);
}

static void TriangleInGold_destructor(TriangleInGold thiz, TriangleInGoldVtable vtab){
    AGILE_LOGD("[%s]", __FUNCTION__);
}

TriangleInGold TriangleInGold_Create(int edge, int color){
    int param[2];

    param[0] = edge;
    param[1] = color;

    return (TriangleInGold) ooc_new( TriangleInGold, (void *)param);
}


int main( int argc, char argv[] ){
    TriangleInGold goldTri;
    Triangle tri;
    Polygon parent;
    
    ooc_init_class(TriangleInGold);

    goldTri = TriangleInGold_Create(3, 1);

    goldTri->bidBasePrice(goldTri, 1888);
    TriangleInGoldVirtual(goldTri)->Triangle.sellToCustomer(ooc_cast(goldTri, Triangle), 1000);

    tri = ooc_cast(goldTri, Triangle);
    TriangleVirtual(tri)->Polygon.getEdges(ooc_cast(tri, Polygon));
    
    parent = ooc_cast(goldTri, Polygon);
    //parent->setColor(parent, 12);
    parent->getColor(parent);
    parent->setOwner(parent, "Yu Jun");

    if ( !PolygonVirtual(parent)->getEdges ){
        AGILE_LOGE("[%s]: PolygonVirtual(parent)->getEdges is NULL", __FUNCTION__);
    }else{
        AGILE_LOGD("before PolygonVirtual(parent)->getEdges()");
        PolygonVirtual(parent)->getEdges(parent);
        AGILE_LOGD("after PolygonVirtual(parent)->getEdges()");
    }
   
    ooc_delete((Object)goldTri);
}

