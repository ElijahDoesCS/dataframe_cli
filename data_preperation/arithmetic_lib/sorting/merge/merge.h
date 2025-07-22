#ifndef MERGE_H
#define MERGE_H

/*
    @param subregion: thread share of requested boundaries that gets modified in place
    @param chunk_size: size of chunk 
 */
void merge_sort(char **subregion, int chunk_size);

#endif // MERGE_H 