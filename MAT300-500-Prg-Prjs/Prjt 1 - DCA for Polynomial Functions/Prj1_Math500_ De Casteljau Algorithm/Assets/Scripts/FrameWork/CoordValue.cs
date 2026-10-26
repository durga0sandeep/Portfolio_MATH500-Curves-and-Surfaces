using UnityEngine;
using TMPro;

public class CoordValue : MonoBehaviour
{

    public TMP_Text TextRef;

    public void UpdateValue(int x, int y)
    {
        TextRef.text = "( " + x + "," + y +" )";
    }

}
