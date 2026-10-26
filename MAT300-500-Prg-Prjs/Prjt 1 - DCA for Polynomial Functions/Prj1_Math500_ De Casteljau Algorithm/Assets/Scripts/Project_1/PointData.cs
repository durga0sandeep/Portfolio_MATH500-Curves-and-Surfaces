using UnityEngine;

public class PointData : MonoBehaviour
{
    //
    //For Dray
    [SerializeField]
    private bool m_Dragging = false;
   
    Vector2 InputVector = new Vector2();

    public int Index = 0;
    public SpriteRenderer Renderer;
    void UpdatePosition()
    {
        if (m_Dragging)
        {
            InputVector = transform.position;
            InputVector.y = SingletonScript.Instance.Camera.ScreenToWorldPoint(Input.mousePosition).y;
            transform.position = InputVector;

            SingletonScript.Instance.curve.ControlPoints[Index] = transform.position;
        }

    }


    private void OnMouseDrag()
    {
        m_Dragging = true;
        UpdatePosition();
    }

    private void OnMouseUp()
    {
        m_Dragging = false;
        Renderer.color = Color.yellow;
    }


    public void ResetColor()
    {
        Renderer.color = Color.white;
        
    }


    private void OnDisable()
    {
        //Test
        //Debug.Log(" " + gameObject.name.ToString());
    }
}
