import QtQuick 1.1

ControlButton {
    image: action.checked ? action.iconChecked : action.icon
    opacity: action.enabled ? 100 : 20
}
