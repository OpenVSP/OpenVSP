varying vec4 frontColor; // color for front face
varying vec4 backColor; // color for back face

void main()
{
  if (gl_FrontFacing) // is the fragment part of a front face?
    {
      gl_FragColor = frontColor;
    }
  else // fragment is part of a back face
    {
      gl_FragColor = backColor;
    }
}
