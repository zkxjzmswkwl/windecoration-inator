use core::time;
use num_traits::PrimInt;
use std::process::exit;
use std::thread;

use winapi::um::winuser::{
    GetAsyncKeyState, GetForegroundWindow, GetWindowLongPtrA, SetWindowLongPtrA, UpdateWindow,
    GWL_STYLE, WS_BORDER, WS_CAPTION, WS_SIZEBOX,
};

unsafe fn remove_window_decorations(style_to_remove: u32) {
    let hwnd = GetForegroundWindow();
    let mut style = GetWindowLongPtrA(hwnd, GWL_STYLE);

    // Alter style bitmask
    style &= !style_to_remove as isize;
    // Nuke the style
    SetWindowLongPtrA(hwnd, GWL_STYLE, style);
    // Force window repaint as to avoid cursor offset issues and apply new style bitmask.
    UpdateWindow(hwnd);
}

fn main() {
    println!("Keybinds:");
    println!("\tLEFT ALT + Q: Toggle decorations on active window\t");
    println!("\tLEFT ALT + Z: Exit program");

    fn msb<N: PrimInt>(n: N) -> N {
        let shift = std::mem::size_of::<N>() * 8 - 1;
        (n >> shift) & N::one()
    }

    loop {
        unsafe {
            // LEFT ALT + Q
            if GetAsyncKeyState(0xA4) == -32767 && GetAsyncKeyState(0x51) == -32767 {
                // Yes, these need to be in this order.
                // And yes, they need to be called individually.
                // Some windows (D3d-11/12 games in particular) freak the fuck out otherwise.
                remove_window_decorations(WS_CAPTION);
                remove_window_decorations(WS_SIZEBOX);
                remove_window_decorations(WS_BORDER);
            }

            // LEFT ALT + Z
            if msb(GetAsyncKeyState(0xA4)) > 0 && msb(GetAsyncKeyState(0x5A)) > 0 & 1 {
                println!("?");
                break;
            }
        }
        thread::sleep(time::Duration::from_millis(20));
    }
}
