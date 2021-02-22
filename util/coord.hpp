#pragma once

#include <ostream>
#include <cstdint>
#include <array>

namespace util
{
    template<typename dimen_t = int32_t> // @assert - dimen_t must be integral type
    struct _coord3D{
        typedef dimen_t type;
        dimen_t mX, mY, mZ;

        inline void operator+=(const _coord3D<dimen_t>& to_add) {
            this->mX += to_add.mX;
            this->mY += to_add.mY;
            this->mZ += to_add.mZ;
        }

        inline _coord3D<dimen_t> operator+(const std::array<dimen_t, 3>& to_add) const {
            return {
                this->mX + to_add[0],
                this->mY + to_add[1],
                this->mZ + to_add[2]
            };
        }

        inline void operator+=(const std::array<dimen_t,3>& to_add) {
            this->mX += to_add[0];
            this->mY += to_add[1];
            this->mZ += to_add[2];
        }

        //inline bool operator==(const _coord<dimen_t>& second) const = default;  // requires C++20
        inline bool operator==(const _coord3D<dimen_t>& second) const {
            return (this->mX == second.mX) &&
                    (this->mY == second.mY) &&
                    (this->mZ == second.mZ);
        }

        inline bool operator<(const _coord3D<dimen_t>& second) const {
            if (this->mX < second.mX)  return true;
            else if (this->mY < second.mY) return true;
            else return this->mZ < second.mZ;
        }

        inline bool operator>(const _coord3D<dimen_t>& second) const {
            return !this->operator==(second) && !this->operator<(second);
        }

        friend std::ostream& operator<<(std::ostream& os, const _coord3D<dimen_t>& coord){
            os << '(';
            if(coord.mX >= 0){
                os << ' ';
                if(coord.mX < 10)
                    os << ' ';
            }else if(coord.mX > -10)
                os << ' ';
            os << coord.mX <<',';

            if(coord.mY >= 0){
                os << ' ';
                if(coord.mY < 10)
                    os << ' ';
            }else if(coord.mY > -10)
                os << ' ';
            os << coord.mY << ',';

            if (coord.mZ >= 0) {
                os << ' ';
                if (coord.mZ < 10)
                    os << ' ';
            }
            else if (coord.mZ > -10)
                os << ' ';
            os << coord.mZ << ')';
            return os;
        }

        _coord3D() noexcept : _coord3D(dimen_t{}, dimen_t{}, dimen_t{}) {}
        _coord3D(dimen_t x, dimen_t y, dimen_t z) noexcept : mX(x), mY(y), mZ(z){}
    };
} // namespace util
