import { Link } from "react-router-dom";
import { useEffect } from 'react'

import styles from './VisHeapNavBar.module.css'

import Box from '@mui/material/Box';
import Grid from '@mui/material/Grid';
import AppBar from '@mui/material/AppBar';
import Toolbar from '@mui/material/Toolbar';
import Divider from '@mui/material/Divider';
import Typography from '@mui/material/Typography';
import Button from '@mui/material/Button';
import IconButton from '@mui/material/IconButton';
import HomeIcon from '@mui/icons-material/Home';
import { createTheme } from '@mui/material/styles';
import { blue, orange, yellow } from '@mui/material/colors';

const BAR_THEME = createTheme({
    palette: {
        primary: {
            main: orange[600],
        },
    },
});


export default function VisHeapNavBar() {

    return (
        <Box>
            <AppBar position="static" theme={BAR_THEME} sx={{fontStyle: 'italic'}}>
                <Toolbar className={styles['container']}>
                    <h1 className={styles['container']}>Visual Heap</h1>
                </Toolbar>
            </AppBar>
        </Box>
    )
}