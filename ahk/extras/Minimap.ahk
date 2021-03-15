;
; Minimap.ahk, 3/5/2021 1:23 PM
;

Class Minimap extends WebGui {

    __new() {
        base.__new("Minimap",, 800, 800)
        this.document.write("
        (%
        <!DOCTYPE html>
        <html>
        <head>
            <style>
                html body { width: 100%; height: 100%; margin: 0px; border: 0px; padding: 0px; }
                canvas { display: block; position: absolute; left: 0; top: 0; right: 0; bottom: 0; } 

                .scene {
                  perspective: 800px;
                  transform-style: preserve-3d;
                  height: 100vh;
                  width: 100vw;
                }

                .minimap {
                  transform-style: preserve-3d;
                  transform: rotateX(38deg) rotateZ(-40deg);
                }
            </style>
        </head>
        <body>
            <canvas>Your browser does not support HTML5 canvas</canvas>
            <script>
                var canvas = document.querySelector('canvas')
                var ctx = canvas.getContext('2d', {alpha:false});
                var cdata = ctx.getImageData(0, 0, canvas.width, canvas.height);

                ctx.clear = function () {
                    this.clearRect(0, 0, canvas.width, canvas.height)
                };

                ctx.setPixel = function (x, y, r, g, b) {
                    var index = (x + y * canvas.width) * 4;
                    cdata.data[index + 0] = r;
                    cdata.data[index + 1] = g;
                    cdata.data[index + 2] = b;
                    cdata.data[index + 3] = 255;
                };

                ctx.fillPixels = function () {
                    ctx.putImageData(cdata, 0, 0);
                };

                window.addEventListener('resize', function () {
                    canvas.width = window.innerWidth
                    canvas.height = window.innerHeight
                    cdata = ctx.getImageData(0, 0, canvas.width, canvas.height);
                });
            </script>
        </body>
        </html>
        )")
        this.document.close()
        this.ctx := this.document.parentWindow.ctx
    }

    draw() {
        this.show(, Format("{} ({})", ptask.areaName, ptask.areaLevel))
        terrian := ptask.getTerrain()
        map := terrian.getMapData(700, 700, 3)
        data := map.data

        loop, % map.height {
            offset := map.width * (A_Index - 1)
            y := map.height - A_Index
            loop, % map.width {
                x := A_Index - 1
                if (NumGet(data+0, offset + (A_Index - 1), "char"))
                    this.ctx.setPixel(x + 50, y + 50, 0xa0, 0xa0, 0xa0)
            }
        }
        this.ctx.fillPixels()
        this.ctx.strokeStyle := "black"
        this.ctx.strokeRect(50.5, 50.5, map.width, map.height)
    }
}

showMinimap() {
    new Minimap().show().draw()
}

Hotkey, !m, showMinimap
