#pragma once

namespace debug{
    namespace connection{
        template<typename Graph_Box_Type>
        bool __IsConncted( const Graph_Box_Type* from, const Graph_Box_Type* to , Direction dir ){
            static_assert( std::is_same_v< std::result_of_t<decltype(Graph_Box_Type::get_adj_box), Direction>, Graph_Box_Type*>,
                "The Graph_Box_Type must have a get_adj_box() function, that takes a direction and returns pointer to Graph_Box_Type" );

            const Graph_Box_Type* temp{ from };

            bool boolean{false}, b2{false};
            for (int i = 0; temp!=nullptr && i < 100; i++){
                if( temp == to ){
                    // clog<<"Connected"<<endl;
                    boolean = true;
                    break;
                }
                temp = temp->get_adj_box(dir);
            }

            dir = connection::__invertDirection(dir);
            temp = to;

            for (int i = 0; temp!=nullptr && i < 100; i++){
                if( temp == from ){
                    // clog<<"Connected Reversed too"<<endl;
                    b2 = true;
                    break;
                }
                temp = temp->get_adj_box(dir);
            }

            // if(!boolean || !b2) clog<<"Not Connected"<<endl;
            return boolean && b2;
        }
    }

    Direction __invertDirection( Direction dir ) noexcept {
        switch (dir){
            case UP:    return DOWN;
            case DOWN:    return UP;
            case RIGHT:    return LEFT;
            case LEFT:    return RIGHT;
            case FRONT_FACING:    return BACK_FACING;
            case BACK_FACING:    return FRONT_FACING;
        }
    }

}
