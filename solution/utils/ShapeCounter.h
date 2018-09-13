//
// Created by lookuut on 12.09.18.
//

#ifndef MADCARS_SHAPECOUNTER_H
#define MADCARS_SHAPECOUNTER_H


class ShapeCounter {

public:
    static ShapeCounter& getInstance()
    {
        static ShapeCounter    instance; // Guaranteed to be destroyed.
        // Instantiated on first use.
        return instance;
    }
private:
    int shape_counter = 0;
    ShapeCounter() {}                    // Constructor? (the {} brackets) are needed here.
    ShapeCounter(ShapeCounter const&);              // Don't Implement
    void operator=(ShapeCounter const&); // Don't implement

public:

    int get_counter() {
        return shape_counter;
    }

    int operator++() {
        return shape_counter++;
    }
};


#endif //MADCARS_SHAPECOUNTER_H
