import React from "react";
import { Paper, Button, Grid } from "@mui/material";

export default function BinChunkVis({binChunk}) {
    return (
        <Paper sx={{padding: "10px"}}>
            Address: {binChunk.address}<br/>
            At bin #{binChunk.bin_index}<br/>
            Chunk size: {binChunk.chunk_size}<br/>
            Type: {binChunk.type}
        </Paper>
    );
}

export function BinChunkVisPopulator({binChunks}) {
    return (
        <Grid container columns={1} spacing={2}>
            {binChunks.map((binChunk, index) => (
                <Grid item key={index}>
                    <BinChunkVis binChunk={binChunk}/>
                </Grid>
            ))}
        </Grid>
    );
}