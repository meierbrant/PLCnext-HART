import { HartDeviceDto } from './hart-device';

export interface HartMuxDto extends HartDeviceDto {
    devices: HartDeviceDto[]
    ipAddress: string
}

export interface HartGw {
    ip: string
    modules: string[]
    serialNo: number
}

export interface HartGwDto {
    gateways: HartGw[]
}

export const hartServerUrl = "http://192.168.254.10:5900"