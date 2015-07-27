# SaturnCDI
Sega Saturn Region Patcher for DiscJuggler CDI Image Format

# Usage:
      SSCdiRP.exe <region string> <cdi image filename>
      SSCdiRP.exe JUE test.cdi

## Arguments Info:
### Image format:
    CDI image file should be made with the option "R-W(CD+G)" checked in Advanced tab of DiscJuggler.
    Or technically says,the image should be 2448 bytes per sector.

### Region string:
    Region string is used to represent the regions you want to patch your exist
    saturn image to.
    It can be any combination of characters in "JTUBKAEL" in any order.
    
    It's case sensitive and the order represent the priority in patching.
    Using JUE means you want to patch the image with Japan support in top
    priority and then USA and EUR support as the lowest priority.
    
    The max count of region that can be applied to the image depends on the 
    original region support count.That means if the unpatched disc contains only
    1 region support.Then even more than 1 region supplied in your command line
    arguments.Only the first 1 will be accepted.
    If the region count provided in arguments is less than the original region
    count.Then region code in original region string will fill the rest place as
    much as possible.
