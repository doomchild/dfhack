#pragma once

typedef vector <df::coord> coord_vec;
class Brush
{
public:
    virtual ~Brush(){};
    virtual coord_vec points(MapExtras::MapCache & mc,DFHack::DFCoord start) = 0;
};
/**
 * generic 3D rectangle brush. you can specify the dimensions of
 * the rectangle and optionally which tile is its 'center'
 */
class RectangleBrush : public Brush
{
public:
    RectangleBrush(int x, int y, int z = 1, int centerx = -1, int centery = -1, int centerz = -1)
    {
        if(centerx == -1)
            cx_ = x/2;
        else
            cx_ = centerx;
        if(centery == -1)
            cy_ = y/2;
        else
            cy_ = centery;
        if(centerz == -1)
            cz_ = z/2;
        else
            cz_ = centerz;
        x_ = x;
        y_ = y;
        z_ = z;
    };
    coord_vec points(MapExtras::MapCache & mc, DFHack::DFCoord start)
    {
        coord_vec v;
        DFHack::DFCoord iterstart(start.x - cx_, start.y - cy_, start.z - cz_);
        DFHack::DFCoord iter = iterstart;
        for(int xi = 0; xi < x_; xi++)
        {
            for(int yi = 0; yi < y_; yi++)
            {
                for(int zi = 0; zi < z_; zi++)
                {
                    if(mc.testCoord(iter))
                        v.push_back(iter);
                    iter.z++;
                }
                iter.z = iterstart.z;
                iter.y++;
            }
            iter.y = iterstart.y;
            iter.x ++;
        }
        return v;
    };
    ~RectangleBrush(){};
private:
    int x_, y_, z_;
    int cx_, cy_, cz_;
};

/**
 * stupid block brush, legacy. use when you want to apply something to a whole DF map block.
 */
class BlockBrush : public Brush
{
public:
    BlockBrush(){};
    ~BlockBrush(){};
    coord_vec points(MapExtras::MapCache & mc, DFHack::DFCoord start)
    {
        coord_vec v;
        DFHack::DFCoord blockc = start / 16;
        DFHack::DFCoord iterc = blockc * 16;
        if( !mc.testCoord(start) )
            return v;
        auto starty = iterc.y;
        for(int xi = 0; xi < 16; xi++)
        {
            for(int yi = 0; yi < 16; yi++)
            {
                v.push_back(iterc);
                iterc.y++;
            }
            iterc.y = starty;
            iterc.x ++;
        }
        return v;
    };
};

/**
 * Column from a position through open space tiles
 * example: create a column of magma
 */
class ColumnBrush : public Brush
{
public:
    ColumnBrush(){};
    ~ColumnBrush(){};
    coord_vec points(MapExtras::MapCache & mc, DFHack::DFCoord start)
    {
        coord_vec v;
        bool juststarted = true;
        while (mc.testCoord(start))
        {
            df::tiletype tt = mc.tiletypeAt(start);
            if(DFHack::LowPassable(tt) || juststarted && DFHack::HighPassable(tt))
            {
                v.push_back(start);
                juststarted = false;
                start.z++;
            }
            else break;
        }
        return v;
    };
};

/**
 * Flood-fill water tiles from cursor (for wclean)
 * example: remove salt flag from a river
 */
class FloodBrush : public Brush
{
public:
    FloodBrush(Core *c){c_ = c;};
    ~FloodBrush(){};
    coord_vec points(MapExtras::MapCache & mc, DFHack::DFCoord start)
    {
        coord_vec v;

        std::stack<DFCoord> to_flood;
        to_flood.push(start);

        std::set<DFCoord> seen;

        while (!to_flood.empty()) {
            DFCoord xy = to_flood.top();
            to_flood.pop();

                        df::tile_designation des = mc.designationAt(xy);

            if (seen.find(xy) == seen.end()
                            && des.bits.flow_size
                            && des.bits.liquid_type == tile_liquid::Water) {
                v.push_back(xy);
                seen.insert(xy);

                maybeFlood(DFCoord(xy.x - 1, xy.y, xy.z), to_flood, mc);
                maybeFlood(DFCoord(xy.x + 1, xy.y, xy.z), to_flood, mc);
                maybeFlood(DFCoord(xy.x, xy.y - 1, xy.z), to_flood, mc);
                maybeFlood(DFCoord(xy.x, xy.y + 1, xy.z), to_flood, mc);

                df::tiletype tt = mc.tiletypeAt(xy);
                if (LowPassable(tt))
                {
                    maybeFlood(DFCoord(xy.x, xy.y, xy.z - 1), to_flood, mc);
                }
                if (HighPassable(tt))
                {
                    maybeFlood(DFCoord(xy.x, xy.y, xy.z + 1), to_flood, mc);
                }
            }
        }

        return v;
    }
private:
    void maybeFlood(DFCoord c, std::stack<DFCoord> &to_flood, MapExtras::MapCache &mc) {
        if (mc.testCoord(c)) {
            to_flood.push(c);
        }
    }
    Core *c_;
};
